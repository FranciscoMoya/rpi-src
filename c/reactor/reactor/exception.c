#include "exception.h"

#define NUM_EXCEPTION_CONTEXTS 4

struct exception_context all_exception_contexts[NUM_EXCEPTION_CONTEXTS];
__thread unsigned current_exception_context = 0;

exception Exception(int ec, const char* what)
{
    exception e = { ec, what };
    return e;
}

