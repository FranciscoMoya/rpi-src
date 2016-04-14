#ifndef EXCEPTION_H
#define EXCEPTION_H

#include <reactor/cexcept.h>

typedef struct {
  int error_code;
  const char* what;
  const char* file;
  unsigned line;
} exception;

define_exception_type(exception);

extern struct exception_context all_exception_contexts[];
extern __thread unsigned current_exception_context;

#define the_exception_context (all_exception_contexts + current_exception_context)
#define Exception(ec,what) exception_init(ec, what, __FILE__, __LINE__)
#define Assert(expr) do { if (!(expr)) Throw Exception(0, "Assertion error"); } while(0)

exception exception_init(int ec,
			 const char* what,
			 const char* file,
			 unsigned line);
void print_exception(exception e);
void print_backtrace(void);

int main(int argc, char* argv[]);
#define main main__

#endif
