#include <reactor/reactor.h>
#include <reactor/input_handler.h>
#include <wiringPi.h>
#include <stdio.h>

static void press(input_handler* ev, int key) { printf("Press %d\n", key); }
static void release(input_handler* ev, int key) { printf("Release %d\n", key); }

int main()
{
    int buttons[] = { 18, 23, 24, 25 };

    wiringPiSetupGpio();
    reactor* r = reactor_new();
    reactor_add(r, (event_handler*) input_handler_new(buttons, 4,
						      press, release));
    reactor_run(r);
}
