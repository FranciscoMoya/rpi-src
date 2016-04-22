CFLAGS=-pthread -std=c11 -Wall -D_POSIX_C_SOURCE=200809L -I../../reactor -ggdb \
	-fasynchronous-unwind-tables 
LDFLAGS=-L../../reactor/reactor -pthread -rdynamic
LDLIBS=-llo -lreactor -lwiringPi -lpthread
CC=gcc

all: piano

clean:
	$(RM) *~ *.o piano
