CFLAGS=-Wall -std=c11 -ggdb
CC=gcc

CSRC=$(wildcard *.c)
OBJS=$(CSRC:%.c=%.o)

all: libreactor.a($(OBJS))

clean:
	$(RM) *~ *.o libreactor.a
