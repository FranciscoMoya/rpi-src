#include <reactor/reactor.h>
#include <reactor/blink_handler.h>
#include <reactor/delayed_handler.h>
#include <wiringPi.h>

void timeout(event_handler* ev)
{
    reactor_add(ev->r, (event_handler*) blink_handler_new(23, 200, 3));
}

void quit(event_handler* ev) { reactor_quit(ev->r); }

int main()
{
    wiringPiSetupGpio();
    reactor* r = reactor_new();
    reactor_add(r, (event_handler*) blink_handler_new(18, 200, 3));
    reactor_add(r, (event_handler*) delayed_handler_new(1500, timeout));
    reactor_add(r, (event_handler*) delayed_handler_new(3000, quit));
    reactor_run(r);
    return 0;
}
