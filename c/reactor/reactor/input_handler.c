#include <reactor/input_handler.h>


input_handler* input_handler_new(int inputs[], size_t ninputs,
				 input_handler_function raising_edge,
				 input_handler_function falling_edge)
{
    return matrix_handler_new(inputs, ninputs,
			      NULL, 0,
			      raising_edge, falling_edge);
}

void input_handler_init(input_handler* h,
			int inputs[], size_t ninputs,
			input_handler_function raising_edge,
			input_handler_function falling_edge)
{
    matrix_handler_init(h, inputs, ninputs,
			NULL, 0,
			raising_edge, falling_edge);
}

void input_handler_destroy(input_handler* h)
{
    event_handler* ev = (event_handler*)h;
    event_handler_destroy(ev);
}
