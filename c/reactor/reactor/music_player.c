#define _GNU_SOURCE
#include <reactor/music_player.h>
#include <reactor/thread_handler_private.h>
#include <reactor/reactor.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>

#define MUSIC_PLAYER "/usr/bin/mpg123","music_player","--loop","-1"

static void* music_player_thread (thread_handler* ev);
static void music_player_init_members (music_player* h, const char* path);

music_player* music_player_new (const char* path)
{
    music_player* h = malloc(sizeof(music_player));
    music_player_init_members(h, path);
    event_handler* ev = (event_handler*) h;
    ev->destroy_self = (event_handler_function) free;
    thread_handler_start (&h->parent, music_player_thread);
    return h;
}

void music_player_init (music_player* h, const char* path)
{
    music_player_init_members(h, path);
    thread_handler_start (&h->parent, music_player_thread);
}


static void music_player_free_members (music_player* h);
static void music_player_find_songs (music_player* h, const char* path);
static void music_player_handler (event_handler* ev);

static void music_player_init_members (music_player* h, const char* path)
{
    thread_handler_init_members(&h->parent, music_player_handler);
    h->current = h->num_songs = 0;
    h->player = h->playing = -1;
    pipe2(h->control, O_DIRECT);
    music_player_find_songs(h, path);
    event_handler* ev = (event_handler*) h;
    h->destroy_parent_members = ev->destroy_members;
    ev->destroy_members = (event_handler_function)music_player_free_members;
}


void music_player_play (music_player* h, int song)
{
    h->current = song;
    write(h->control[1], &h->current, sizeof(h->current));
}


void music_player_stop (music_player* h)
{
    music_player_play (h, -1);

    event_handler* ev = (event_handler*) h;
    while (h->playing >= 0)
	reactor_demultiplex_events(ev->r);
}


void music_player_next (music_player* h)
{
    int next = h->current + 1;
    if (next >= h->num_songs)
        next -= h->num_songs;

    music_player_play (h, next);
}


void music_player_prev (music_player* h)
{
    int prev = h->current - 1;
    if (prev < 0)
        prev += h->num_songs;

    music_player_play (h, prev);
}


static void music_player_add_song (music_player* h, const char* song);

static void music_player_find_songs (music_player* h, const char* path)
{
    DIR* d = opendir(path);

    if (! d)
        return;

    for(;;) {
        struct dirent* entry = readdir(d);

        if (!entry) break;

        char song[PATH_MAX];
        sprintf(song, "%s/%s", path, entry->d_name);

        struct stat stbuf;
        stat(song, &stbuf);

        if (!S_ISDIR(stbuf.st_mode))
            music_player_add_song(h, song);
        else if (strcmp(entry->d_name, "..")
                 && strcmp (entry->d_name, "."))
            music_player_find_songs (h, song);
    }

    closedir(d);
}


static void music_player_add_song (music_player* h, const char* song)
{
    if (h->num_songs >= MUSIC_PLAYER_MAX_SONGS)
        return;

    h->songs[h->num_songs++] = strdup(song);
}


void music_player_destroy (music_player* h)
{
    event_handler* ev = (event_handler*) h;
    event_handler_destroy(ev);
}


static void music_player_free_members (music_player* h)
{
    music_player_stop(h);
    for (int i = 0; i < h->num_songs; ++i)
        free(h->songs[i]);
    h->destroy_parent_members((event_handler*)h);
    close(h->control[0]);
    close(h->control[1]);
}


static void player_start (music_player* h, const char* song);
static void player_stop (music_player* h);

static void music_player_handler (event_handler* ev)
{
    int song;
    pipe_handler_read((pipe_handler*)ev, &song, sizeof(song));

    music_player* h = (music_player*)ev;
    h->playing = song;
}

static void* music_player_thread (thread_handler* h)
{
    music_player* mp = (music_player*) h;

    while(!h->cancel) {
	int song;
	if (0 > read (mp->control[0], &song, sizeof(song)))
	    break;

	player_stop(mp);
	if (song >= 0)
	    player_start(mp, mp->songs[song]);
    }
    return NULL;
}


static void player_start (music_player* h, const char* song)
{
    h->player = fork();
    if (!h->player) {
	int out = open("/dev/null", O_WRONLY);
	dup2(out, 1);
	dup2(1, 2);
        execlp(MUSIC_PLAYER, song, 0);
    }
}


static void player_stop (music_player* h)
{
    if (h->player < 0) return;

    kill(h->player, SIGTERM);
    waitpid(h->player, NULL, 0);
    h->player = -1;
}
