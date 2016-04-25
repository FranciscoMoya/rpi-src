#ifndef VIDEO_PLAYER_H
#define VIDEO_PLAYER_H

#include <reactor/process_handler.h>

typedef struct video_player_ video_player;
struct video_player_ {
    process_handler parent;
    event_handler_function destroy_parent_members;
};

typedef enum {
    VP_SPEED_LESS     = '1',
    VP_SPEED_MORE     = '2',
    VP_REWIND         = '<',
    VP_FAST_FORWARD   = '>',
    VP_INFO           = 'z',
    VP_AUDIO_PREV     = 'j',
    VP_AUDIO_NEXT     = 'k',
    VP_CHAPTER_PREV   = 'i',
    VP_CHAPTER_NEXT   = 'o',
    VP_SUB_PREV       = 'n',
    VP_SUB_NEXT       = 'm',
    VP_SUB_TOGGLE     = 's',
    VP_SUB_SHOW       = 'w',
    VP_SUB_HIDE       = 'x',
    VP_SUB_DELAY_LESS = 'd',
    VP_SUB_DELAY_MORE = 'f',
    VP_EXIT           = 'q',
    VP_PAUSE          = 'p',
    VP_VOLUME_LESS    = '-',
    VP_VOLUME_MORE    = '+',
    VP_30S_REW        = 0x445b1b, // \e[D
    VP_30S_FWD        = 0x435b1b, // \e[C
    VP_600S_REW       = 0x425b1b, // \e[B
    VP_600S_FWD       = 0x415b1b, // \e[A
} video_player_command;


video_player* video_player_new(const char* path);
void video_player_init(video_player* this, const char* path);
void video_player_destroy(video_player* this);
void video_player_send(video_player* this, video_player_command cmd);

#endif
