#include "analog_handler.h"
#include <reactor/thread_handler_private.h>
#include <reactor/exception.h>
#include <wiringPi.h>
#include <wiringPiSPI.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>

static void* analog_handler_thread(thread_handler* this);
static void analog_handler_init_members (analog_handler* this,
					 int pin, int low, int high,
					 analog_handler_function low_handler,
					 analog_handler_function high_handler);
static void analog_handler_poll(analog_handler* h);
static void analog_handler_handler(event_handler* ev);
static void do_nothing(analog_handler* this, unsigned v) {}


analog_config DEFAULT_ANALOG_CONFIG = {
    .low_handler = do_nothing, .high_handler = do_nothing,
    .low = 0, .high = 0,
    .buf_size = 4096,
    .clk_freq = 500000,
    .pin = 0
};


analog_handler* analog_handler_new (analog_config* cfg)
{
    analog_handler* h = malloc(sizeof(analog_handler));
    analog_handler_init_members(h, cfg);
    event_handler* ev = (event_handler*) h;
    ev->destroy_self = (event_handler_function) free;
    thread_handler_start (&h->parent, analog_handler_thread);
    return h;
}


void analog_handler_init (analog_handler* this, analog_config* cfg)
{
    analog_handler_init_members(this, cfg);
    thread_handler_start (&this->parent, analog_handler_thread);
}


void analog_handler_destroy (analog_handler* this)
{
    event_handler_destroy((event_handler*)this);
}


static void* analog_handler_thread(thread_handler* h)
{
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
					 analog_config* cfg)
{
    thread_handler_init_members(&this->parent, analog_handler_handler);
    this->cfg = *cfg;
    this->current = (cfg->low + cfg->high)/2;
    this->spi = wiringPiSPISetup(0, cfg->clk_freq);
    this->buf = malloc(cfg->buf_size);

    event_handler* ev = (event_handler*) this;
    this->destroy_parent_members = ev->destroy_members;
    ev->destroy_members = (event_handler_function) analog_handler_free_members;
}


static void analog_handler_handler(event_handler* ev)
{
    int input;
    if (0 > read(ev->fd, &input, sizeof(input)))
	Throw Exception(errno, "No pudo leer");

    analog_handler* h = (analog_handler*) ev;
    analog_config* cfg = &h->cfg;
    if (input > cfg->high)
	cfg->high_handler(h, input);
    else
	cfg->low_handler(h, input);
}


static void analog_handler_measure(analog_handler* this);

static void analog_handler_poll(analog_handler* this)
{
    analog_config* cfg = &this->cfg;
    struct timespec t = { .tv_sec = 1, .tv_nsec = 0 };
    pinMode(cfg->pin, OUTPUT);
    digitalWrite(cfg->pin, 0);
    nanosleep(&t, NULL);
    pinMode(this->cfg.pin, INPUT);
    wiringPiSPIDataRW(0, this->buf, BUF_SIZE);
    analog_handler_measure(this);
}


static int raise_bit(int c)
{
    for(int i=0; i<8; ++i,c>>=1)
	if (c&1) return i;
    Throw Exception(0, "Could not find any bit set to one");
}


static int raise_point(const unsigned char* buf, int low, int high)
{
    if (high - low <= 1)
	return low + raise_bit(buf[low]);

    int guess = (low + high)/2;
    if (buf[guess])
	return raise_point(buf, low, guess);
    else
	return raise_point(buf, guess, low);
}


static void check_ends(const unsigned char* buf, size_t size)
{
    if (buf[size-1] == 0)
	Throw Exception(0, "Buffer too small");

    if (this->buf[0] == 0xff)
	Throw Exception(0, "Capacitor was not discharged");
}


static void analog_handler_measure(analog_handler* this)
{
    check_ends(this->buf, BUF_SIZE);
	
    int prev = this->current;
    this->current = raise_point(this->buf, 0, BUF_SIZE);

    pipe_handler* ph = (pipe_handler*) this;
    analog_config* cfg = &this->cfg;
    if (this->current >= cfg->high && prev < cfg->high)
	pipe_handler_write_ne(ph, &this->current, sizeof(this->current));
    else if (this->current <= cfg->low && prev > cfg->low)
	pipe_handler_write_ne(ph, &this->current, sizeof(this->current));
}
