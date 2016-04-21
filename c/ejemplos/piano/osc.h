#ifndef OSC_H
#define OSC_H

#include <stdarg.h>
#include <stddef.h>

size_t osc_encode_message(char* buf, size_t size,
			  const char* cmd, va_list* ap);
size_t osc_decode_message(const char* in, size_t size_in,
			  char* out, size_t size_out);
int osc_is_async(const char* command);
int osc_is_done(const char* command);

#endif

