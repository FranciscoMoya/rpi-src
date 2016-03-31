CFLAGS=-pthread -std=c11 -Wall -D_POSIX_C_SOURCE=200809L -ggdb -I..
CC=gcc

CSRC=$(wildcard *.c)
OBJS=$(CSRC:%.c=%.o)

all: libreactor.a($(OBJS))

clean:
	$(RM) *~ *.o libreactor.a
