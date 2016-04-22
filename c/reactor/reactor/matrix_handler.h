#ifndef MATRIX_HANDLER_H
#define MATRIX_HANDLER_H

/* Manejador de entradas digitales con debouncing automático.  Se
   configuran automáticamente como entradas con pull-down. Se consulta
   periódicamente para detectar cambios.
 */

#include <reactor/thread_handler.h>
#include <time.h>

typedef struct matrix_handler_ matrix_handler;
typedef void (*matrix_handler_function)(matrix_handler* h, int key);
struct matrix_handler_ {
    thread_handler parent;
    struct timespec period;
    int *rows;
    int *cols;
    int *values;
    size_t nrows, ncols;
    matrix_handler_function raising_edge;
    matrix_handler_function falling_edge;
    event_handler_function destroy_parent_members;
};

matrix_handler* matrix_handler_new(int rows[], size_t nrows,
				   int cols[], size_t ncols,
				   matrix_handler_function raising_edge,
				   matrix_handler_function falling_edge);
void matrix_handler_init(matrix_handler* h,
			 int rows[], size_t nrows,
			 int cols[], size_t ncols,
			 matrix_handler_function raising_edge,
			 matrix_handler_function falling_edge);

void matrix_handler_destroy(matrix_handler* h);

#endif
