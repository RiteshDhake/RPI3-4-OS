#pragma once

#include "common.h"
#include "framebuffer.h"
#include"font.h"


#define MAX_TEXT_OBJECTS 1024
#define MAX_TEXT_LENGTH 128

void video_mark_dirty();
u32 video_get_frame_count();
u32 video_get_frame_time();
void video_render_frame();

void video_clear_all_text();
void video_remove_text(u32 id);
void video_set_text_visible(u32 id, bool visible);
void video_move_text(u32 id, u32 x, u32 y);
void video_update_text(u32 id, char *new_text);
u32 video_add_text(char *text, u32 x, u32 y, u32 color);
void demo_usage();

extern bool frame_dirty;
extern u32 frame_count;
extern u64 last_frame_time;