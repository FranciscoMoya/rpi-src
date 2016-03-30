CFLAGS=-std=c11 -Wall -D_POSIX_C_SOURCE -ggdb -I..
LDFLAGS=-L../reactor
LDLIBS=-lreactor
CC=gcc

CSRC=$(wildcard *.c)
OBJS=$(CSRC:%.c=%.o)
EXES=$(CSRC:%.c=%)

all: $(EXES)

%: %.o

clean:
	$(RM) *~ *.o $(EXES)
