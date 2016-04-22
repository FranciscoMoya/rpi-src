#ifndef INPUT_HANDLER_H
#define INPUT_HANDLER_H

/* Manejador de entradas digitales con debouncing automático.  Se
   configuran automáticamente como entradas con pull-down. Se consulta
   periódicamente para detectar cambios.
 */

#include <reactor/matrix_handler.h>

typedef matrix_handler input_handler;
typedef matrix_handler_function input_handler_function;

input_handler* input_handler_new(int inputs[], size_t ninputs,
				 input_handler_function raising_edge,
				 input_handler_function falling_edge);
void input_handler_init(input_handler* h,
			int inputs[], size_t ninputs,
			input_handler_function raising_edge,
			input_handler_function falling_edge);

void input_handler_destroy(input_handler* h);

#endif
