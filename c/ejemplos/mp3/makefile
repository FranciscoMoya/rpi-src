CFLAGS=-pthread -std=c11 -Wall -D_POSIX_C_SOURCE=200809L -I../../reactor -ggdb \
	-fasynchronous-unwind-tables 
LDFLAGS=-L../../reactor/reactor -pthread -rdynamic
LDLIBS=-lreactor -lwiringPi -lpthread
CC=gcc

all: test_video_player

test_video_player: test_video_player.o video_player.o

clean:
	$(RM) *~ *.o mp3 test_video_player
