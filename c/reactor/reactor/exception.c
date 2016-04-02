#include "exception.h"
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <execinfo.h>

#define NUM_EXCEPTION_CONTEXTS 4
#define MAX_CALLTRACE_LEN 10

struct exception_context all_exception_contexts[NUM_EXCEPTION_CONTEXTS];
__thread unsigned current_exception_context = 0;

__thread void* exception_calltrace[MAX_CALLTRACE_LEN];
__thread int exception_calltrace_len;
__thread const char* exception_file;
__thread unsigned exception_line;

exception exception_init(int ec, const char* what,
			 const char* file, unsigned line)
{
    exception e = { ec, what };
    exception_file = file;
    exception_line = line;
    exception_calltrace_len = backtrace(exception_calltrace, MAX_CALLTRACE_LEN);
    return e;
}

void print_exception(exception e)
{
    fprintf(stderr, "Uncaught exception (error_code %d) at %s:%d\n",
	    e.error_code, exception_file, exception_line);
    perror(e.what);
    print_backtrace();
}

void print_backtrace(void)
{
    fprintf(stderr, "Current call trace (last %d):\n",
	    exception_calltrace_len);
    backtrace_symbols_fd(exception_calltrace,
			 exception_calltrace_len,
			 2);
}

#undef main

int main__(int argc, char* argv[]) __attribute__((weak));
int main(int argc, char* argv[]) __attribute__((weak));

int main(int argc, char* argv[])
{
    exception e;
    Try {
	return main__(argc, argv);
    }
    Catch (e) print_exception(e);
}
