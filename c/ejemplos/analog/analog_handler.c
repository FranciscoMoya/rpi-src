#include "analog_handler.h"
#include <reactor/thread_handler_private.h>
#include <stdlib.h>
#include <time.h>

#define SPI_CLOCK_FREQ 500000
#define BUF_SIZE 4096

static void* analog_handler_thread(thread_handler* this);
static void analog_handler_init_members (analog_handler* this,
					 int pin, int low, int high,
					 analog_handler_function low_handler,
					 analog_handler_function high_handler);
static void analog_handler_poll(analog_handler* h);
static struct timespec remaining_time (const struct timespec* period,
				       const struct timespec* start,
				       const struct timespec* end);

analog_handler* analog_handler_new (int pin, int low, int high,
				    analog_handler_function low_handler,
				    analog_handler_function high_handler)
{
    analog_handler* h = malloc(sizeof(analog_handler));
    analog_handler_init_members(h, pin, low, high, low_handler, high_handler);
    event_handler* ev = (event_handler*) h;
    ev->destroy_self = (event_handler_function) free;
    thread_handler_start (&h->parent, analog_handler_thread);
    return h;
}


void analog_handler_init (analog_handler* this,
			  int pin, int low, int high,
			  analog_handler_function low_handler,
			  analog_handler_function high_handler)
{
    analog_handler_init_members(this, pin, low, high, low_handler, high_handler);
    thread_handler_start (&this->parent, analog_handler_thread);
}


void analog_handler_destroy (analog_handler* this)
{
    event_handler_destroy((event_handler*)this);
}


static void* analog_handler_thread(thread_handler* h)
{
    struct timespec start, end;
    analog_handler* in = (analog_handler*) h;
    while(!h->cancel)
	analog_handler_poll(in);
    return NULL;
}


static void analog_handler_free_members(analog_handler* this)
{
    this->destroy_parent_members((event_handler*)this);
    free(this->buf);
}


static void analog_handler_init_members (analog_handler* this,
					 int pin, int low, int high,
					 analog_handler_function low_handler,
					 analog_handler_function high_handler)
{
    this->pin = pin; this->low = low; this->high = high;
    this->low_handler = low_handler; this->high_handler = high_handler;
    this->current = (low + high)/2;
    this->spi = wiringPiSPISetup(0, SPI_CLOCK_FREQ);
    this->buf = malloc(BUF_SIZE);

    event_handler* ev = (event_handler*) this;
    this->destroy_parent_members = ev->destroy_members;
    ev->destroy_members = (event_handler_function) analog_handler_free_members;
}


static void analog_handler_measure(analog_handler* this);

static void analog_handler_poll(analog_handler* this)
{
    struct timespec t = { .tv_sec = 1, .tv_nsec = 0 };
    pinMode(this->pin, OUTPUT);
    digitalWrite(pin, 0);
    nanosleep(&t, NULL);
    pinMode(this->pin, INPUT);
    wiringPiSPIDataRW(0, this->buf, BUF_SIZE);
    analog_handler_measure(this);
}


static int raise_bit(int c)
{
    for(int i=0; i<8; ++i,c>>=1)
	if (c&1) return i;
    Throw Exception(0, "Could not find any bit set to one");
}


static int raise_point(const char* buf, int low, int high)
{
    if (high - low <= 1)
	return low + raise_bit(buf[low]);

    int guess = (low + high)/2;
    if (buf[guess])
	return raise_point(buf, low, guess);
    else
	return raise_point(buf, guess, low);
}


static void analog_handler_measure(analog_handler* this)
{
    int prev = this->current;
    this->current = raise_point(this->buf, 0, BUF_SIZE);
}
