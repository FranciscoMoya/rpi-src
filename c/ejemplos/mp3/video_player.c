#include "video_player.h"
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

video_player* video_player_new(const char* path)
{
    video_player* this = malloc(sizeof(video_player));
    event_handler* ev = (event_handler*) this;
    video_player_init(this, path);
    ev->destroy_self = (event_handler_function) free;
    return this;
}


static void video_player_start(video_player* this, const char* path);
static void video_player_free_members(video_player* this);
static void wait_seconds(int sec);
static void do_nothing(event_handler* ev) {}

void video_player_init(video_player* this, const char* path)
{
    event_handler* ev = (event_handler*) this;
    process_handler_init(&this->parent, do_nothing, do_nothing);
    if (process_handler_is_child(&this->parent))
	video_player_start(this, path);
    this->destroy_parent_members = ev->destroy_members;
    ev->destroy_members = (event_handler_function) video_player_free_members;
}


void video_player_destroy(video_player* this)
{
    event_handler_destroy((event_handler*)this);
}


void video_player_send(video_player* this, video_player_command cmd)
{
    char buf[4];
    *(unsigned*)buf = cmd;
    Assert(0 < write(this->parent.out, buf, strlen(buf)));
}


static void video_player_start(video_player* this, const char* path)
{
    event_handler* ev = (event_handler*)this;
    dup2(open("/dev/null", O_WRONLY), 1);
    dup2(1, 2);
    dup2(ev->fd, 0);
    execlp("/usr/bin/omxplayer", "omxplayer", path, NULL);
    exit(0);
}


static void video_player_free_members(video_player* this)
{
    video_player_send(this, VP_EXIT);
    wait_seconds(1);
    this->destroy_parent_members((event_handler*)this);
}


static void wait_seconds(int sec)
{
    struct timespec t = { sec, 0 };
    nanosleep(&t, NULL);
}
