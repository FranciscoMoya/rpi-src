#ifndef BLINK_HANDLER_H
#define BLINK_HANDLER_H

/* Manejador de salidas digitales que genera un parpadeo de duración
   controlada.  Al terminar eleva una excepción para que se elimine de
   forma automática.
 */

#include <reactor/thread_handler.h>

typedef struct blink_handler_ blink_handler;
struct blink_handler_ {
    thread_handler parent;
    int output;
    int period;
    int nblinks;
};

blink_handler* blink_handler_new(int output, int period, size_t nblinks);
void blink_handler_init(blink_handler* h,
			int output, int period, size_t nblinks);
void blink_handler_destroy(blink_handler* h);

#endif
