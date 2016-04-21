CFLAGS=-pthread -std=c11 -Wall -D_POSIX_C_SOURCE=200809L -I../../reactor -ggdb \
	-fasynchronous-unwind-tables 
LDFLAGS=-L../../reactor/reactor -pthread -rdynamic
LDLIBS=-lreactor -lwiringPi -lpthread
CC=gcc

all: test_synth_handler test_osc

piano: piano.o synth_handler.o osc.o
test_osc: test_osc.o osc.o
test_osc_synth: test_osc_synth.o osc.o

test_synth_handler: CFLAGS+=-DNDEBUG
test_synth_handler: test_synth_handler.o synth_handler.o osc.o

clean:
	$(RM) *~ *.o piano test_osc
