#include <reactor/exception.h>
#include <reactor/thread_handler_private.h>
#include <reactor/matrix_handler.h>
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

static void matrix_handler_init_members(matrix_handler* h,
				       int rows[], size_t nrows,
				       int cols[], size_t ncols,
				       matrix_handler_function raising_edge,
				       matrix_handler_function falling_edge);
static void* matrix_handler_thread(thread_handler* h);
static void matrix_handler_handler(event_handler* ev);
static void matrix_handler_free_members (matrix_handler* h);


matrix_handler* matrix_handler_new(int rows[], size_t nrows,
				 int cols[], size_t ncols,
				 matrix_handler_function raising_edge,
				 matrix_handler_function falling_edge)
{
    matrix_handler* h = malloc(sizeof(matrix_handler));
    matrix_handler_init_members(h, rows, nrows, cols, ncols,
				raising_edge, falling_edge);
    event_handler* ev = (event_handler*) h;
    ev->destroy_self = (event_handler_function) free;
    thread_handler_start (&h->parent, matrix_handler_thread);
    return h;
}


void matrix_handler_init(matrix_handler* h,
			int rows[], size_t nrows,
			int cols[], size_t ncols,
			matrix_handler_function raising_edge,
			matrix_handler_function falling_edge)
{
    matrix_handler_init_members(h, rows, nrows, cols, ncols,
				raising_edge, falling_edge);
    thread_handler_start (&h->parent, matrix_handler_thread);
}


void matrix_handler_destroy(matrix_handler* h)
{
    event_handler* ev = (event_handler*)h;
    event_handler_destroy(ev);
}


static void matrix_handler_init_gpio(matrix_handler* h)
{
    for(int i = 0; i < h->nrows; ++i) {
	int pin = h->rows[i];
	pinMode(pin, INPUT);
	pullUpDnControl(pin, PUD_DOWN);
    }

    for(int i = 0; i < h->ncols; ++i) {
	int pin = h->cols[i];
	pinMode(pin, OUTPUT);
	digitalWrite(pin, 0);
    }
}


static void matrix_handler_init_members(matrix_handler* h,
				       int rows[], size_t nrows,
				       int cols[], size_t ncols,
				       matrix_handler_function raising_edge,
				       matrix_handler_function falling_edge)
{
    thread_handler_init_members(&h->parent, matrix_handler_handler);
    struct timespec period = {.tv_sec = 0, .tv_nsec = 50000000 }; /* 50ms */
    h->period = period;
    h->nrows = nrows;
    h->ncols = ncols;
    h->raising_edge = raising_edge;
    h->falling_edge = falling_edge;
    h->rows = calloc(nrows, sizeof(int));
    h->cols = calloc(ncols, sizeof(int));
    h->values = calloc(nrows*ncols, sizeof(int));
    memcpy(h->rows, rows, nrows * sizeof(int));
    memcpy(h->cols, cols, ncols * sizeof(int));
    matrix_handler_init_gpio(h);
    
    event_handler* ev = (event_handler*) h;
    h->destroy_parent_members = ev->destroy_members;
    ev->destroy_members = (event_handler_function) matrix_handler_free_members;
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


static void matrix_handler_poll_input(matrix_handler* h, int i, int j)
{
    int key = i + j * h->nrows;
    int value = h->values[key];
    int new_value = digitalRead(h->rows[i]);
    if (new_value == value)
	return;
    h->values[key] = new_value;
    if (!new_value)
	key = -key;
    pipe_handler* ph = (pipe_handler*) h;
    pipe_handler_write_ne(ph, &key, sizeof(key));
}



static void matrix_handler_poll_column(matrix_handler* h, int j)
{
    for (int i = 0; i < h->nrows; ++i)
	matrix_handler_poll_input(h, i, j);
}


static void matrix_handler_poll(matrix_handler* h)
{
    if (h->ncols == 0) {
	matrix_handler_poll_column(h, 0);
	return;
    }
    
    for (int j = 0; j < h->ncols; ++j) {
	digitalWrite(h->cols[(j-1) % h->ncols], 0);
	digitalWrite(h->cols[j], 1);
	matrix_handler_poll_column(h, j);
    }
}

static void* matrix_handler_thread(thread_handler* h)
{
    struct timespec start, end;
    matrix_handler* ih = (matrix_handler*) h;
    clock_gettime(CLOCK_REALTIME, &start);
    while(!h->cancel) {
	matrix_handler_poll(ih);
	clock_gettime(CLOCK_REALTIME, &end);
	struct timespec t = remaining_time(&ih->period, &start, &end);
	nanosleep(&t, NULL);
	start = end;
    }
    return NULL;
}

static void matrix_handler_handler(event_handler* ev)
{
    int input;
    if (0 > read(ev->fd, &input, sizeof(input)))
	Throw Exception(errno, "No pudo leer");

    matrix_handler* h = (matrix_handler*) ev;
    if (input > 0)
	h->raising_edge(h, input);
    else
	h->falling_edge(h, -input);
}


static void matrix_handler_free_members (matrix_handler* h)
{
    h->destroy_parent_members((event_handler*)h);
    free(h->rows);
    free(h->cols);
    free(h->values);
}
