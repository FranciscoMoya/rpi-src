#include "analog_handler.h"
#include <reactor/reactor.h>
#include <wiringPi.h>
#include <stdio.h>

static void bajo(analog_handler* this, unsigned v)
{
    puts("Bajo limite inferior");
}
		 
static void alto(analog_handler* this, unsigned v)
{
    puts("Sobre limite superior");
}

int main(int argc, char* argv[])
{
    wiringPiSetupGpio();
    reactor* r = reactor_new();
    analog_config cfg = DEFAULT_ANALOG_CONFIG;
    cfg.pin = 25; cfg.low = 100; cfg.high = 200;
    cfg.low_handler = bajo; cfg.high_handler = alto;
    analog_handler* in = analog_handler_new(&cfg);
    reactor_add(r, (event_handler*)in);
    reactor_run(r);
    reactor_destroy(r);
    return 0;
}
