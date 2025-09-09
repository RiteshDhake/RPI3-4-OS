#pragma once

#include "common.h"

void video_set_resolution(u32 xres, u32 yres, u32 bpp);
void video_draw_char(char c, u32 pox_x, u32 pos_y);
void video_draw_string(char *s, u32 pos_x, u32 pos_y);
void video_draw_pixel(u32 x, u32 y, u32 color);
void video_init();
void video_set_dma(bool b);
void demo_usage();
void video_mark_dirty();
u32 video_get_frame_count();
u32 video_get_frame_time();
void video_render_frame();
void video_draw_string_colored(char *s, u32 pos_x, u32 pos_y, u32 text_color, u32 bg_color);
void video_draw_char_colored(char c, u32 pos_x, u32 pos_y, u32 text_color, u32 bg_color);
void video_clear_all_text();
void video_remove_text(u32 id);
void video_set_text_visible(u32 id, bool visible);
void video_move_text(u32 id, u32 x, u32 y);
void video_update_text(u32 id, char *new_text);
u32 video_add_text(char *text, u32 x, u32 y, u32 color);



//comes from fontData.c
u32 font_get_height();
u32 font_get_width();
bool font_get_pixel(char ch, u32 x, u32 y);