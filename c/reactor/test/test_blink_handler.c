#include <reactor/reactor.h>
#include <reactor/blink_handler.h>
#include <wiringPi.h>

int main()
{
    wiringPiSetupGpio();
    reactor* r = reactor_new();
    reactor_add(r, (event_handler*) blink_handler_new(18, 200, 5));
//    reactor_add(r, (event_handler*) blink_handler_new(27, 200, 5));
//    reactor_add(r, (event_handler*) blink_handler_new(22, 200, 5));
    reactor_run(r);
}
