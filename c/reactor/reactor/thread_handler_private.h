#ifndef THREAD_HANDLER_PRIVATE_H
#define THREAD_HANDLER_PRIVATE_H

/* Interfaz privada usada exclusivamente por clases derivadas.
*/

#include <reactor/thread_handler.h>

void thread_handler_init_members (thread_handler* h,
				  event_handler_function handler);

/* Puede elevar excepción y se llama en constructor.  Debe retrasarse
   hasta el momento en que puede destruirse sin problemas. */
void thread_handler_start (thread_handler* h, thread_handler_function thread);

/* Llamado en destructor, no eleva excepciones */
void thread_handler_stop (thread_handler* h);

#endif
