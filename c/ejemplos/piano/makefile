CFLAGS=-pthread -std=c11 -Wall -D_POSIX_C_SOURCE=200809L -I.. -ggdb \
	-fasynchronous-unwind-tables
LDFLAGS=-L../../reactor -pthread -rdynamic
LDLIBS=-lreactor -lwiringPi -lpthread
CC=gcc

CSRC=piano.c synth_handler.c
OBJS=$(CSRC:%.c=%.o)

all: piano
piano: $(CSRC)

clean:
	$(RM) *~ *.o piano

