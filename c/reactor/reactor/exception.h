#ifndef EXCEPTION_H
#define EXCEPTION_H

#include <reactor/cexcept.h>

typedef struct { int error_code; const char* what; } exception;
define_exception_type(exception);
extern struct exception_context all_exception_contexts[];
extern __thread unsigned current_exception_context;
#define the_exception_context (all_exception_contexts + current_exception_context)
exception Exception(int ec, const char* what);
#define Assert(expr) do { if (!(expr)) Throw Exception(0, "Assertion error"); } while(0)

#endif
