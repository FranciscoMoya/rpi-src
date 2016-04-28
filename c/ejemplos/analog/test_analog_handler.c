#include "analog_handler.h"
#include <reactor/reactor.h>
#include <wiringPi.h>
#include <stdio.h>

static void bajo(analog_handler* this)
{
    puts("Bajo limite inferior");
}
		 
static void alto(analog_handler* this)
{
    puts("Sobre limite superior");
}

int main(int argc, char* argv[])
{
    wiringPiSetupGpio();
    reactor* r = reactor_new();
    analog_handler* in = analog_handler_new(25, 100, 200,
					    bajo, alto);
    reactor_add(r, (event_handler*)in);
    reactor_run(r);
    reactor_destroy(r);
    return 0;
}
