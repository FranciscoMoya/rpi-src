#ifndef THREAD_HANDLER_H
#define THREAD_HANDLER_H

/* Especialización de pipe_handler. Ejecuta un hilo en paralelo que
   usa una pipe para comunicar datos.  Puede usarse para generar
   eventos periódicos o retardados sin necesidad de periodic.
*/

#include <reactor/pipe_handler.h>
#include <pthread.h>

typedef struct thread_handler_ thread_handler;
typedef void* (thread_handler_function)(thread_handler*);
struct thread_handler_ {
    pipe_handler parent;
    pthread_t thread;
    int cancel;
    event_handler_function destroy_parent_members;
};

thread_handler* thread_handler_new (event_handler_function handler,
				    thread_handler_function thread);
void thread_handler_init (thread_handler* h,
			  event_handler_function handler,
			  thread_handler_function thread);
void thread_handler_destroy (thread_handler* h);

#endif
