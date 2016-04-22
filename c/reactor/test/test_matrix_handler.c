#include <reactor/reactor.h>
#include <reactor/matrix_handler.h>
#include <wiringPi.h>
#include <stdio.h>

static void press(matrix_handler* ev, int key) { printf("Press %d\n", key); }
static void release(matrix_handler* ev, int key) { printf("Release %d\n", key); }

#define NELEMS(x) (sizeof(x)/sizeof(x[0]))

int main()
{
    int rows[] = { 4, 17, 27, 22 };
    int cols[] = { 18, 23, 24, 25 };

    wiringPiSetupGpio();
    reactor* r = reactor_new();
    reactor_add(r, (event_handler*) matrix_handler_new(rows, NELEMS(rows),
						       cols, NELEMS(cols),
						       press, release));
    reactor_run(r);
    return 0;
}
