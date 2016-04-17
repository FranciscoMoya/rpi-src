CFLAGS=-pthread -std=c11 -Wall -D_POSIX_C_SOURCE=200809L -I../../reactor -ggdb \
	-fasynchronous-unwind-tables -DNDEBUG
LDFLAGS=-L../../reactor/reactor -pthread -rdynamic
LDLIBS=-lreactor -lwiringPi -lpthread
CC=gcc

all: piano test_osc

piano: piano.o synth_handler.o osc.o
test_osc: test_osc.o osc.o

clean:
	$(RM) *~ *.o piano test_osc
