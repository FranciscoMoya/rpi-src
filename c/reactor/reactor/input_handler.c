#include <reactor/reactor.h>
#include <reactor/input_handler.h>
#include <reactor/thread_handler_private.h>
#include <wiringPi.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>

/* El modo en que wiringPi trata las ISR de un pin GPIO las hace
   virtualmente inútiles.  La demultiplexión no puede realizarse de
   forma automática salvo que hubiera diferentes funciones para cada
   pin.

   Interesa desacoplar la detección de los cambios de la generación de
   eventos. Tanto para debouncing como para limitar la tasa de eventos
   externos para acotar la interferencia. Por tanto polling es
   prácticamente equivalente a ISR en este caso.
*/

static void input_handler_init_members(input_handler* h,
				       int inputs[], size_t ninputs,
				       input_handler_function raising_edge,
				       input_handler_function falling_edge);
static void* input_handler_thread(thread_handler* h);
static void input_handler_handler(event_handler* ev);
static void input_handler_free_members (input_handler* h);


input_handler* input_handler_new(int inputs[], size_t ninputs,
				 input_handler_function raising_edge,
				 input_handler_function falling_edge)
{
    input_handler* h = malloc(sizeof(input_handler));
    input_handler_init_members(h, inputs, ninputs, raising_edge, falling_edge);
    event_handler* ev = (event_handler*) h;
    ev->destroy_self = (event_handler_function) free;
    thread_handler_start (&h->parent, input_handler_thread);
    return h;
}

void input_handler_init(input_handler* h,
			int inputs[], size_t ninputs,
			input_handler_function raising_edge,
			input_handler_function falling_edge)
{
    input_handler_init_members(h, inputs, ninputs, raising_edge, falling_edge);
    thread_handler_start (&h->parent, input_handler_thread);
}

void input_handler_destroy(input_handler* h)
{
    event_handler* ev = (event_handler*)h;
    event_handler_destroy(ev);
}

static void input_handler_init_inputs(input_handler* h)
{
    for(int i = 0; i < h->ninputs; ++i) {
	int pin = h->inputs[i];
	pinMode(pin, INPUT);
	pullUpDnControl(pin, PUD_DOWN);
    }
}


static void input_handler_init_members(input_handler* h,
				       int inputs[], size_t ninputs,
				       input_handler_function raising_edge,
				       input_handler_function falling_edge)
{
    thread_handler_init_members(&h->parent, input_handler_handler);
    struct timespec period = {.tv_sec = 0, .tv_nsec = 50000000 }; /* 50ms */
    h->period = period;
    h->ninputs = ninputs;
    h->raising_edge = raising_edge;
    h->falling_edge = falling_edge;
    h->inputs = calloc(ninputs, sizeof(int));
    h->values = calloc(ninputs, sizeof(int));
    memcpy(h->inputs, inputs, ninputs * sizeof(int));
    input_handler_init_inputs(h);
    
    event_handler* ev = (event_handler*) h;
    h->destroy_parent_members = ev->destroy_members;
    ev->destroy_members = (event_handler_function) input_handler_free_members;
}

struct timespec remaining_time (const struct timespec* period,
				const struct timespec* start,
				const struct timespec* end)
{
    struct timespec t = *period;
    t.tv_sec -= (end->tv_sec - start->tv_sec);
    t.tv_nsec -= (end->tv_nsec - start->tv_nsec);
    while (t.tv_nsec < 0) {
	--t.tv_sec; t.tv_nsec += 1000000000;
    }
    if (t.tv_sec < 0) {
	t.tv_sec = t.tv_nsec = 0;
    }
    return t;
}


static void input_handler_poll_input(input_handler* h, int i)
{
    int pin = h->inputs[i];
    int value = h->values[i];
    int new_value = digitalRead(pin);
    if (new_value == value)
	return;
    h->values[i] = new_value;
    if (!new_value)
	pin = -pin;
    pipe_handler* ph = (pipe_handler*) h;
    pipe_handler_write_ne(ph, &pin, sizeof(pin));
}


static void input_handler_poll(input_handler* h)
{
    for (int i = 0; i < h->ninputs; ++i)
	input_handler_poll_input(h, i);
}

static void* input_handler_thread(thread_handler* h)
{
    struct timespec start, end;
    input_handler* ih = (input_handler*) h;
    clock_gettime(CLOCK_REALTIME, &start);
    while(!h->cancel) {
	input_handler_poll(ih);
	clock_gettime(CLOCK_REALTIME, &end);
	struct timespec t = remaining_time(&ih->period, &start, &end);
	nanosleep(&t, NULL);
	start = end;
    }
    return NULL;
}

static void input_handler_handler(event_handler* ev)
{
    int input;
    if (0 > read(ev->fd, &input, sizeof(input)))
	Throw Exception(errno, "No pudo leer");

    input_handler* h = (input_handler*) ev;
    if (input > 0)
	h->raising_edge(h, input);
    else
	h->falling_edge(h, -input);
}


static void input_handler_free_members (input_handler* h)
{
    h->destroy_parent_members((event_handler*)h);
    free(h->inputs);
    free(h->values);
}
