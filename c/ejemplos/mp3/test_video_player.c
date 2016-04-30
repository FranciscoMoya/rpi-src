#include <reactor/reactor.h>
#include <reactor/console.h>
#include <reactor/exception.h>
#include <unistd.h>
#include "video_player.h"

static video_player_command read_key(int fd);

int main(int argc, char* argv[])
{
    void* state = console_set_raw_mode(0);
    reactor* r = reactor_new();
    
    video_player* video = video_player_new(argv[1]);
    void keyboard(event_handler* ev) {
        int key = read_key(ev->fd);
        if ('q' == key)
            reactor_quit(r);
        else
	    video_player_send(video, key);
    }

    reactor_add(r, event_handler_new(0, keyboard));
    reactor_add(r, (event_handler*)video);
    reactor_run(r);
    reactor_destroy(r);
    console_restore(0, state);
    return 0;
}


static video_player_command read_key(int fd)
{
    char buf[4] = {0};
    if (0 > read(fd, buf, sizeof(buf)))
	Throw Exception(0, "Failed read!");
    return *(video_player_command*)buf;
}
