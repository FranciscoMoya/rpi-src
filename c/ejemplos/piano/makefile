CFLAGS=-pthread -std=c11 -Wall -D_POSIX_C_SOURCE=200809L -I../../reactor -ggdb \
	-fasynchronous-unwind-tables 
LDFLAGS=-L../../reactor/reactor -pthread -rdynamic
LDLIBS=-llo -lreactor -lwiringPi -lpthread
CC=gcc

all: test_synth_handler

piano: piano.o synth_handler.o osc.o

test_synth_handler: CFLAGS+=-DNDEBUG
test_synth_handler: test_synth_handler.o synth_handler.o

clean:
	$(RM) *~ *.o piano test_synth_handler
