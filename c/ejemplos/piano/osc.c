#include "osc.h"
#include <reactor/exception.h>
#include <string.h>
#include <sys/time.h>
#include <arpa/inet.h>

/* Esto no es una implementación completa de OSC ni pretende
   serlo. Solo codificamos y decodificamos los mensajes que usamos y
   con restricciones.
 */

static struct {
    const char* name;
    const char* format;
    int args;
    int async;
} cmd[] = {
    { "#bundle",       NULL,         0, 0 },
    { "/quit",         ",",          0, 0 },
    { "/status",       ",",          0, 1 },
    { "/status.reply", ",iiiiiffdd", 0, 0 },
    { "/clearSched",   ",",          0, 0 },
    { "/notify",       ",i",         0, 1 },
    { "/dumpOSC",      ",i",         0, 0 },
    { "/sync",         ",i",         0, 1 },
    { "/synced",       ",i",         0, 0 },
    { "/b_allocRead",  ",isii",      0, 1 },
    { "/b_query",      ",i",         0, 1 },
    { "/b_info",       ",iiif",      0, 0 },
    { "/g_freeAll",    ",i",         0, 0 },
    { "/g_new",        ",iii",       0, 0 },
    { "/d_recv",       ",bi",        0, 1 },
    { "/d_loadDir",    ",s",         0, 1 },
    { "/d_load",       ",s",         0, 1 },
    { "/n_free",       ",i",         0, 0 },
    { "/n_go",         ",iiiiiii",   0, 0 },
    { "/n_set",        ",i",         1, 0 },
    { "/n_end",        ",iiiii",     0, 0 },
    { "/s_new",        ",siii",      1, 0 },
    { "/done",         ",s",         0, 0 },
    { "/fail",         ",ss",        0, 0 },
    { NULL,            NULL,         0, 0 }
};

static const char* replies[] = {
    "/done",
    "/fail",
    NULL
};

static char* encode_str0(char* buf, size_t size, const char* s);
static char* encode_str(char* buf, size_t size, va_list* ap);
static char* encode_int(char* buf, size_t size, va_list* ap);
static char* encode_int0(char* buf, size_t size, unsigned v);
static char* encode_float(char* buf, size_t size, va_list* ap);
static char* encode_blob(char* buf, size_t size, va_list* ap);
static char* encode_double(char* buf, size_t size, va_list* ap);
static char* encode_timestamp(char* p, size_t size);

static struct {
    char code;
    char* (*encode)(char* buf, size_t size, va_list* ap);
} types[] = {
    { 's', encode_str },
    { 'i', encode_int },
    { 'f', encode_float },
    { 'd', encode_double },
    { 'b', encode_blob },
    { '\0', NULL }
};


static int format(const char* cmd, char* format, va_list ap);
static char* encode(char* buf, size_t size, char type, va_list* ap);

size_t osc_encode_bundle(char* buf, size_t size, va_list* ap);

size_t osc_encode_message(char* buf, size_t size, const char* cmd, va_list* ap)
{
    if (0 == strcmp(cmd, "#bundle"))
	return osc_encode_bundle(buf, size, ap);

    memset(buf, 0, size);
    char* p = buf;
    p = encode_str0(p, size - (p-buf), cmd);
    char fmt[32];
    if (format(cmd, fmt, *ap))
	va_arg(*ap, char*);
    p = encode_str0(p, size - (p-buf), fmt);
    char type, *fmtstr = fmt;
    while ((type = *++fmtstr))
	p = encode(p, size - (p-buf), type, ap);
    return p - buf;
}

size_t osc_encode_bundle(char* buf, size_t size, va_list* ap)
{
    char* p = buf;
    p = encode_str0(p, size - (p-buf), "#bundle");
    p = encode_timestamp(p, size - (p-buf));
    for(;;) {
	const char* cmd = va_arg(*ap, const char*);
	if (NULL == cmd) break;
	p += 4;
	size_t n = osc_encode_message(p, size - (p-buf), cmd, ap);
	encode_int0(p - 4, 4, n);
	p += n;
    }
    return p - buf;
}

static size_t copy_int(void* dst, const void* orig);
static size_t copy_string(void* dst, const void* orig);
static size_t skip_string(const void* orig);
static size_t copy_blob(void* dst, const void* orig);

size_t osc_decode_message(const char* in, size_t size_in,
			  char* out, size_t size_out)
{
    const char* pi = in;
    char* po = out;
    size_t n = copy_string(po, pi);
    pi += n; po += n;
    const char* format = pi;
    pi += skip_string(pi);
    char type;
    while((type = *++format)) {
	if (type == 's')
	    n = copy_string(po, pi);
	else if (type == 'b')
	    n = copy_blob(po, pi);
	else if (type == 'i')
	    n = copy_int(po, pi);
	else if (type == 'f')
	    n = copy_int(po, pi);
	else if (type == 'd') {
	    n = copy_int(po + 4, pi) + copy_int(po, pi + 4);
	}
	pi += n; po += n;
    }
    return po - out;
}


int osc_is_async(const char* command)
{
    for(int i=0; NULL != cmd[i].name ; ++i)
	if (0 == strcmp(command, cmd[i].name))
	    return cmd[i].async;
    Throw Exception(0, "Unknown OSC command");
}


int osc_is_done(const char* command)
{
    for(int i=0; NULL != replies[i] ; ++i)
	if (0 == strcmp(command, replies[i]))
	    return 1;
    return 0;
}


static void append_args(char* format, va_list ap)
{
    char type;
    char* args = va_arg(ap, char*);
    format += strlen(format);
    while ((type = *args++)) {
	*format++ = 's';
	*format++ = type;
    }
    *format = '\0';
}


static int format(const char* command, char* format, va_list ap)
{
    for(int i=0; NULL != cmd[i].name ; ++i)
	if (0 == strcmp(command, cmd[i].name)) {
	    strcpy(format, cmd[i].format);
	    if (cmd[i].args)
		append_args(format, ap);
	    return cmd[i].args;
	}
    Throw Exception(0, "Unknown OSC command");
}


static char* encode(char* buf, size_t size, char type, va_list* ap)
{
    for(int i=0; '\0' != types[i].code ; ++i)
	if (type == types[i].code)
	    return types[i].encode(buf, size, ap);
    Throw Exception(0, "Unknown OSC command");
}


static size_t padding(size_t len)
{
    size_t pad = len % 4;
    return pad? 4 - pad: 0;
}


static char* encode_str(char* buf, size_t size, va_list* ap)
{
    const char* str = va_arg(*ap, char*);
    return encode_str0(buf, size, str);
}


static char* encode_str0(char* buf, size_t size, const char* s)
{
    size_t len = strlen(s) + 1;
    if (len > size)
	Throw Exception(0, "Insuficient buffer space to fit string");
    memcpy(buf, s, len);
    return buf + len + padding(len);
}


static char* encode_int(char* buf, size_t size, va_list* ap)
{
    unsigned v = va_arg(*ap, unsigned);
    return encode_int0(buf, size, v);
}


static char* encode_int0(char* buf, size_t size, unsigned v)
{
    if (4 > size)
	Throw Exception(0, "Insuficient buffer space to fit int");

    v = htonl(v);
    memcpy(buf, &v, 4);
    return buf + 4;
}


static char* encode_float(char* buf, size_t size, va_list* ap)
{
    float v = va_arg(*ap, double);
    return encode_int0(buf, size, *(unsigned*)&v);
}


static char* encode_double(char* buf, size_t size, va_list* ap)
{
    double v = va_arg(*ap, double);
    char* p = encode_int0(buf, size, *(1+(unsigned*)&v));
    return encode_int0(p, size-4, *(unsigned*)&v);
}


static char* encode_blob(char* buf, size_t size, va_list* ap)
{
    void* blob = va_arg(*ap, void*);
    size_t len = va_arg(*ap, size_t);
    if (len + 4 > size)
	Throw Exception(0, "Insuficient buffer space to fit blob");
    
    char* p = encode_int0(buf, size, len);
    memcpy(p, blob, len);
    return p + len + padding(len);
}


static char* encode_timestamp(char* buf, size_t size)
{
    // timestamp: 1 -> As soon as possible
    char* p = encode_int0(buf, size, 0);
    return encode_int0(p, size - (p-buf), 1);
}


static size_t copy_int(void* dst, const void* orig)
{
    unsigned v = ntohl(*(unsigned*)orig);
    *(unsigned*)dst = v;
    return 4;
}


static size_t copy_string(void* dst, const void* orig)
{
    size_t len = strlen((char*)orig) + 1;
    memcpy(dst, orig, len);
    return len + padding(len);
}


static size_t skip_string(const void* orig)
{
    size_t len = strlen((char*)orig) + 1;
    return len + padding(len);
}


static size_t copy_blob(void* dst, const void* orig)
{
    copy_int(dst, orig);
    size_t len = *(unsigned*)dst;
    memcpy(dst + 4, orig + 4, len);
    return len + 4 + padding(len);
}
