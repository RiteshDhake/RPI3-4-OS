#include "mailbox.h"
#include "printf.h"
#include "timer.h"
#include "Graphics/font.h"
#include "dma.h"
#include "mm.h"
#include <stddef.h>

#define TEXT_COLOR 0xFFFFFFFF
#define BACK_COLOR 0xFF1E1E2E

#define MB (1024 * 1024)


#define MAX_CHARS 256
static u32 *glyph_cache_32bpp[MAX_CHARS];
static u8 *glyph_cache_8bpp[MAX_CHARS];
static bool cache_initialized = false;

typedef struct {
    u32 x1, y1, x2, y2;
    bool dirty;
} dirty_rect;

static dirty_rect screen_dirty = {0, 0, 0, 0, false};


void init_glyph_cache() {
    if (cache_initialized) return;
    
    u32 glyph_size_32 = font_get_width() * font_get_height() * sizeof(u32);
    u32 glyph_size_8 = font_get_width() * font_get_height();
    
    // Allocate cache memory (you'll need a proper allocator here)
    u8 *cache_mem = (u8*)(VB_MEM_LOCATION + (4 * MB)); // After video buffer
    
    for (int c = 0; c < MAX_CHARS; c++) {
        glyph_cache_32bpp[c] = (u32*)(cache_mem + c * glyph_size_32);
        glyph_cache_8bpp[c] = (u8*)(cache_mem + (MAX_CHARS * glyph_size_32) + c * glyph_size_8);
        
        // Pre-render all characters
        for (int y = 0; y < font_get_height(); y++) {
            for (int x = 0; x < font_get_width(); x++) {
                bool pixel = font_get_pixel(c, x, y);
                int idx = y * font_get_width() + x;
                
                glyph_cache_32bpp[c][idx] = pixel ? TEXT_COLOR : BACK_COLOR;
                glyph_cache_8bpp[c][idx] = pixel ? 2 : 1; // palette indices
            }
        }
    }
    cache_initialized = true;
}




void video_draw_char_colored(char c, u32 pos_x, u32 pos_y, u32 text_color, u32 bg_color) {
    if (!cache_initialized) return;
    if (pos_x + font_get_width() > fb_req.res.xres || 
        pos_y + font_get_height() > fb_req.res.yres) return;
    
    unsigned char uc = (unsigned char)c;
    if (uc >= MAX_CHARS) uc = '?';
    
    if (fb_req.depth.bpp == 32) {
        u32 pitch_words = fb_req.pitch.pitch >> 2;
        
        for (int y = 0; y < font_get_height(); y++) {
            u32 *dest = (u32 *)DRAWBUFFER + (pos_y + y) * pitch_words + pos_x;
            
            for (int x = 0; x < font_get_width(); x++) {
                bool pixel = font_get_pixel(c, x, y);
                dest[x] = pixel ? text_color : bg_color;
            }
        }
    } else if (fb_req.depth.bpp == 8) {
        for (int y = 0; y < font_get_height(); y++) {
            u8 *dest = (u8 *)DRAWBUFFER + (pos_y + y) * fb_req.pitch.pitch + pos_x;
            
            for (int x = 0; x < font_get_width(); x++) {
                bool pixel = font_get_pixel(c, x, y);
                dest[x] = pixel ? (text_color & 0xFF) : (bg_color & 0xFF);
            }
        }
    }
}

void video_draw_string_colored(char *s, u32 pos_x, u32 pos_y, u32 text_color, u32 bg_color) {
    u32 x = pos_x;
    int len = 0;
    while (s[len]) len++;
    
    for (int i = 0; i < len; i++) {
        video_draw_char_colored(s[i], x, pos_y, text_color, bg_color);
        x += font_get_width() + 2;
        
        if (x + font_get_width() >= fb_req.res.xres) {
            x = pos_x;
            pos_y += font_get_height() + 2;
        }
    }
}


void video_draw_char(char c, u32 pos_x, u32 pos_y) {
    if (!cache_initialized) return;
    if (pos_x + font_get_width() > fb_req.res.xres || 
        pos_y + font_get_height() > fb_req.res.yres) return;
    
    unsigned char uc = (unsigned char)c;
    if (uc >= MAX_CHARS) uc = '?';
    
    if (fb_req.depth.bpp == 32) {
        // OPTIMIZATION: Direct memory copy without DMA for characters
        u32 *src = glyph_cache_32bpp[uc];
        u32 pitch_words = fb_req.pitch.pitch >> 2;
        
        for (int y = 0; y < font_get_height(); y++) {
            u32 *dest = (u32 *)DRAWBUFFER + (pos_y + y) * pitch_words + pos_x;
            u32 *src_line = src + y * font_get_width();
            
            // Unrolled copy for common font widths
            int width = font_get_width();
            while (width >= 4) {
                dest[0] = src_line[0];
                dest[1] = src_line[1];
                dest[2] = src_line[2];
                dest[3] = src_line[3];
                dest += 4;
                src_line += 4;
                width -= 4;
            }
            while (width--) *dest++ = *src_line++;
        }
    } else if (fb_req.depth.bpp == 8) {
        u8 *src = glyph_cache_8bpp[uc];
        
        for (int y = 0; y < font_get_height(); y++) {
            u8 *dest = (u8 *)DRAWBUFFER + (pos_y + y) * fb_req.pitch.pitch + pos_x;
            u8 *src_line = src + y * font_get_width();
            
            // Fast byte copy
            int width = font_get_width();
            while (width >= 4) {
                dest[0] = src_line[0];
                dest[1] = src_line[1];
                dest[2] = src_line[2];
                dest[3] = src_line[3];
                dest += 4;
                src_line += 4;
                width -= 4;
            }
            while (width--) *dest++ = *src_line++;
        }
    }
}

void video_draw_string(char *s, u32 pos_x, u32 pos_y) {
    u32 x = pos_x;
    
    // OPTIMIZATION: Calculate string length once
    int len = 0;
    while (s[len]) len++;
    
    // Draw all characters without intermediate operations
    for (int i = 0; i < len; i++) {
        video_draw_char(s[i], x, pos_y);
        x += font_get_width() + 2;
        
        if (x + font_get_width() >= fb_req.res.xres) {
            x = pos_x;
            pos_y += font_get_height() + 2;
        }
    }
}
