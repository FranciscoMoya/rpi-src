#ifndef MUSIC_PLAYER_H
#define MUSIC_PLAYER_H

/* Clase derivada de process_handler que funciona como music player.  El
   lado del proceso hijo queda oculto al usuario.
*/

#include <reactor/thread_handler.h>
#include <sys/types.h>

#define MUSIC_PLAYER_MAX_SONGS 32

typedef struct music_player_ music_player;
struct music_player_ {
    thread_handler parent;
    int current, num_songs;
    char* songs[MUSIC_PLAYER_MAX_SONGS];
    int playing, control[2];
    pid_t player;
    event_handler_function destroy_parent_members;
};

music_player* music_player_new (const char* path);
void music_player_init (music_player* ev, const char* path);
void music_player_destroy (music_player* ev);
void music_player_play (music_player* ev, int song);
void music_player_stop (music_player* ev);
void music_player_next (music_player* ev);
void music_player_prev (music_player* ev);

#endif
