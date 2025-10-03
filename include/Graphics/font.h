#pragma once

#include "common.h"
#include "framebuffer.h"

u32 font_get_height();
u32 font_get_width();
bool font_get_pixel(char ch, u32 x, u32 y);
void video_draw_string_colored(char *s, u32 pos_x, u32 pos_y, u32 text_color, u32 bg_color);
void video_draw_char_colored(char c, u32 pos_x, u32 pos_y, u32 text_color, u32 bg_color);
void video_draw_char(char c, u32 pos_x, u32 pos_y);
void video_draw_string(char *s, u32 pos_x, u32 pos_y);
void init_glyph_cache();