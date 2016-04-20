#ifndef OSC_H
#define OSC_H

#include <stdarg.h>
#include <stddef.h>

int osc_has_reply(const char* command);
int osc_is_notified(const char* command);
int osc_is_reply(const char* command);
size_t osc_encode_message(char* buf, size_t size,
			  const char* cmd, va_list* ap);
size_t osc_decode_message(const char* in, size_t size_in,
			  char* out, size_t size_out);

#endif

