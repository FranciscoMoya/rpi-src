CFLAGS=-pthread -std=c11 -Wall -D_POSIX_C_SOURCE=200809L -I.. -ggdb \
	-fasynchronous-unwind-tables
LDFLAGS=-L../reactor -pthread -rdynamic
LDLIBS=-lreactor -lwiringPi -lpthread
CC=gcc

CSRC=$(wildcard *.c)
OBJS=$(CSRC:%.c=%.o)
EXES=$(CSRC:%.c=%)

all: $(EXES)

%: %.o

clean:
	$(RM) *~ *.o $(EXES)
