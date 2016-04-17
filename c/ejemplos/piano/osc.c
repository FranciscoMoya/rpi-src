#include "osc.h"
#include <reactor/exception.h>
#include <string.h>
#include <arpa/inet.h>

/* Esto no es una implementación completa de OSC ni pretende
   serlo. Solo codificamos y decodificamos los mensajes que usamos y
   con restricciones:

   - Mensajes de carga de SynthDefs.

   - Mensaje s_new asumiendo argumentos enteros si los hay.

   - Mensaje n_free con un solo argumento.

   - Notificación done y fail.
 */

static struct {
    const char* name;
    const char* format;
    int args;
} cmd[] = {
    { "/quit", ",", 0 },
    { "/d_recv", ",bb", 0 },
    { "/d_load", ",sb", 0 },
    { "/d_loadDir", ",sb", 0 },
    { "/n_free", ",i", 0 },
    { "/s_new", ",siii", 1 },
    { "/done", ",s", 0 },
    { "/fail", ",ss", 0 },
    { NULL, NULL }
};

static char* encode_str0(char* buf, size_t size, const char* s);
static char* encode_str(char* buf, size_t size, va_list* ap);
static char* encode_int(char* buf, size_t size, va_list* ap);
static char* encode_int0(char* buf, size_t size, unsigned v);
static char* encode_blob(char* buf, size_t size, va_list* ap);

static struct {
    char code;
    char* (*encode)(char* buf, size_t size, va_list* ap);
} types[] = {
    { 's', encode_str },
    { 'i', encode_int },
    { 'b', encode_blob },
    { '\0', NULL }
};


static void format(const char* cmd, char* format, va_list ap);
static char* encode(char* buf, size_t size, char type, va_list* ap);


size_t osc_encode_message(char* buf, size_t size, const char* cmd, va_list ap)
{
    memset(buf, 0, size);
    
    char* p = buf + 4;
    p = encode_str0(p, size - (p-buf), cmd);
    char fmt[16];
    format(cmd, fmt, ap);
    p = encode_str0(p, size - (p-buf), fmt);
    char type, *fmtstr = fmt;
    while ((type = *++fmtstr))
	p = encode(p, size - (p-buf), type, &ap);
    encode_int0(buf, 4, p - buf - 4);
    return p - buf;
}

static size_t copy_int(void* dst, const void* orig);
static size_t copy_string(void* dst, const void* orig);
static size_t skip_string(const void* orig);
static size_t copy_blob(void* dst, const void* orig);

void osc_decode_message(const char* in, size_t size_in,
			char* out, size_t size_out)
{
    size_t len;
    char* po = out;
    const char* pi = in + copy_int(&len, in);
    if (len > size_in - 4)
	Throw Exception(0, "Inconsistent OSC message size");
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
	pi += n; po += n;
    }
}

static void append_args(char* format, va_list ap)
{
    char type;
    while((type = *++format))
	if (type == 's' || type == 'b')
	    va_arg(ap, char*);
	else if (type == 'i')
	    va_arg(ap, unsigned);
    for(;;) {
	if (NULL == va_arg(ap, char*)) break;
	va_arg(ap, unsigned);
	*format++ = 's';
	*format++ = 'i';
    }
    *format = '\0';
}


static void format(const char* command, char* format, va_list ap)
{
    for(int i=0; NULL != cmd[i].name ; ++i)
	if (0 == strcmp(command, cmd[i].name)) {
	    strcpy(format, cmd[i].format);
	    if (cmd[i].args)
		append_args(format, ap);
	    return;
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
