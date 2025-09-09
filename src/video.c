#include "mailbox.h"
#include "printf.h"
#include "timer.h"
#include "video.h"
#include "dma.h"
#include "mm.h"
#include <stddef.h>

// Copy up to n characters from src to dest.
// If src is shorter than n, pad the rest with '\0'.
// Returns dest.
char *strncpy(char *dest, const char *src, size_t n) {
    size_t i = 0;

    // Copy characters until we hit end of src or n
    for (; i < n && src[i] != '\0'; i++) {
        dest[i] = src[i];
    }

    // Pad remaining space with '\0'
    for (; i < n; i++) {
        dest[i] = '\0';
    }

    return dest;
}

// #include <stddef.h>

// Compare two strings.
// Returns:
//   0  if s1 == s2
//   <0 if s1 < s2
//   >0 if s1 > s2
int strcmp(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return (unsigned char)(*s1) - (unsigned char)(*s2);
}

typedef struct {
    mailbox_tag tag;
    u32 xres;
    u32 yres;
} mailbox_fb_size;

typedef struct {
    mailbox_tag tag;
    u32 bpp;
} mailbox_fb_depth;

typedef struct {
    mailbox_tag tag;
    u32 pitch;
} mailbox_fb_pitch;

typedef struct {
    mailbox_tag tag;
    u32 base; 
    u32 screen_size;
} mailbox_fb_buffer;

typedef struct {
    mailbox_fb_size res;
    mailbox_fb_size vres; //virtual resolution..
    mailbox_fb_depth depth;
    mailbox_fb_buffer buff;
    mailbox_fb_pitch pitch;
} mailbox_fb_request;

static mailbox_fb_request fb_req;

static dma_channel *dma;
static u8 *vid_buffer;

static u32 *bg32_buffer;
static u32 *bg8_buffer;


#define MAX_TEXT_OBJECTS 128
#define MAX_TEXT_LENGTH 128

typedef struct {
    char text[MAX_TEXT_LENGTH];
    u32 x, y;
    u32 color;
    bool visible;
    bool dirty;  // Needs redraw
    u32 id;     // Unique identifier
} text_object;

static text_object text_objects[MAX_TEXT_OBJECTS];
static int num_text_objects = 0;
static u32 next_text_id = 1;

// FRAME BUFFER MANAGEMENT
static bool frame_dirty = false;
static u32 frame_count = 0;
static u64 last_frame_time = 0;

#define TEXT_COLOR 0xFFFFFFFF
#define BACK_COLOR 0xFF0055BB

#define MB (1024 * 1024)

//hack for not having an allocate function yet...
#define UNCACHED_MEM_BASE 0x40000000
#define BG32_MEM_LOCATION (LOW_MEMORY + (10 * MB))
#define BG8_MEM_LOCATION (BG32_MEM_LOCATION + (10 * MB))
#define VB_MEM_LOCATION (BG8_MEM_LOCATION + (4 * MB))

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

void video_init() {
    dma = dma_open_channel(CT_NORMAL);
    vid_buffer = (u8 *)VB_MEM_LOCATION;

    printf("DMA CHANNEL: %d\n", dma->channel);
    printf("VID BUFF: %X\n", vid_buffer);

    bg32_buffer = (u32 *)BG32_MEM_LOCATION;
    bg8_buffer = (u32 *)BG8_MEM_LOCATION;

    for (int i=0; i<(10 * MB) / 4; i++) {
        bg32_buffer[i] = BACK_COLOR;
    }

    for (int i=0; i<(4 * MB) / 4; i++) {
        bg8_buffer[i] = 0x01010101;
    }
    init_glyph_cache();
    
    // Initialize text system
    num_text_objects = 0;
    frame_dirty = true;
}

static bool use_dma = true;

#define BUS_ADDR(x) (((u64)x | 0x40000000) & ~0xC0000000)
#define FRAMEBUFFER ((volatile u8 *)BUS_ADDR(fb_req.buff.base))
#define DMABUFFER ((volatile u8 *)vid_buffer)
#define DRAWBUFFER (use_dma ? DMABUFFER : FRAMEBUFFER)

void video_set_dma(bool b) {
    use_dma = b;
}

void do_dma(void *dest, void *src, u32 total) {
    // Optimized: Use larger chunks and fewer DMA operations
    const u32 max_chunk = 0x3FFFFF; // Even larger chunks
    u32 start = 0;
    
    while (total > 0) {
        u32 num_bytes = (total > max_chunk) ? max_chunk : total;
        
        dma_setup_mem_copy(dma, dest + start, src + start, num_bytes, 8); // 64-bit transfers
        dma_start(dma);
        dma_wait(dma);
        
        start += num_bytes;
        total -= num_bytes;
    }
}

void video_dma() {
    do_dma(FRAMEBUFFER, DMABUFFER, fb_req.buff.screen_size);
}

static bool screen_initialized = false;

void clear_screen_once() {
    if (screen_initialized) return;
    
    if (fb_req.depth.bpp == 32) {
        u32 *buff = (u32 *)DRAWBUFFER;
        for (int i = 0; i < fb_req.buff.screen_size / 4; i++) {
            buff[i] = BACK_COLOR;
        }
    } else if (fb_req.depth.bpp == 8) {
        u32 *buff = (u32 *)DRAWBUFFER;
        for (int i = 0; i < fb_req.buff.screen_size / 4; i++) {
            buff[i] = 0x01010101;
        }
    }
    
    screen_initialized = true;
}

typedef struct  {
    mailbox_tag tag;
    u32 offset;
    u32 num_entries;
    u32 entries[8];
} mailbox_set_palette;


// Add text to screen - returns ID for later updates
u32 video_add_text(char *text, u32 x, u32 y, u32 color) {
    if (num_text_objects >= MAX_TEXT_OBJECTS) return 0;
    
    text_object *obj = &text_objects[num_text_objects++];
    strncpy(obj->text, text, MAX_TEXT_LENGTH - 1);
    obj->text[MAX_TEXT_LENGTH - 1] = 0;// strings should be null terminated for printf function
    obj->x = x;
    obj->y = y;
    obj->color = color;
    obj->visible = true;
    obj->dirty = true;
    obj->id = next_text_id++;
    
    frame_dirty = true;
    return obj->id;
}

// Update existing text
void video_update_text(u32 id, char *new_text) {
    for (int i = 0; i < num_text_objects; i++) {
        if (text_objects[i].id == id) {
            if (strcmp(text_objects[i].text, new_text) != 0) {
                strncpy(text_objects[i].text, new_text, MAX_TEXT_LENGTH - 1);
                text_objects[i].text[MAX_TEXT_LENGTH - 1] = 0;
                text_objects[i].dirty = true;
                frame_dirty = true;
            }
            break;
        }
    }
}

// Move text to new position
void video_move_text(u32 id, u32 x, u32 y) {
    for (int i = 0; i < num_text_objects; i++) {
        if (text_objects[i].id == id) {
            if (text_objects[i].x != x || text_objects[i].y != y) {
                text_objects[i].x = x;
                text_objects[i].y = y;
                text_objects[i].dirty = true;
                frame_dirty = true;
            }
            break;
        }
    }
}

// Hide/show text
void video_set_text_visible(u32 id, bool visible) {
    for (int i = 0; i < num_text_objects; i++) {
        if (text_objects[i].id == id) {
            if (text_objects[i].visible != visible) {
                text_objects[i].visible = visible;
                text_objects[i].dirty = true;
                frame_dirty = true;
            }
            break;
        }
    }
}

// Remove text
void video_remove_text(u32 id) {
    for (int i = 0; i < num_text_objects; i++) {
        if (text_objects[i].id == id) {
            // Shift remaining objects down
            for (int j = i; j < num_text_objects - 1; j++) {
                text_objects[j] = text_objects[j + 1];
            }
            num_text_objects--;
            frame_dirty = true;
            break;
        }
    }
}

// Clear all text
void video_clear_all_text() {
    if (num_text_objects > 0) {
        num_text_objects = 0;
        frame_dirty = true;
    }
}

// Keep your optimized drawing functions but make them work with colors
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

// OPTIMIZED FRAME RENDERING
void video_render_frame() {
    if (!frame_dirty) return; // Skip if nothing changed
    
    u64 ms_start = timer_get_ticks() / 1000;
    
    // Clear background (keeping your optimization)
    if (fb_req.depth.bpp == 32) {
        if (use_dma) {
            do_dma((void*)vid_buffer, bg32_buffer, fb_req.buff.screen_size);
        } else {
            u32 *dest = (u32 *)FRAMEBUFFER;
            u32 *src = bg32_buffer;
            u32 count = fb_req.buff.screen_size / 4;
            while (count >= 4) {
                dest[0] = src[0]; dest[1] = src[1]; 
                dest[2] = src[2]; dest[3] = src[3];
                dest += 4; src += 4; count -= 4;
            }
            while (count--) *dest++ = *src++;
        }
    } else if (fb_req.depth.bpp == 8) {
        if (use_dma) {
            do_dma((void*)vid_buffer, bg8_buffer, fb_req.buff.screen_size);
        } else {
            u32 *dest = (u32 *)FRAMEBUFFER;
            u32 *src = bg8_buffer;
            u32 count = fb_req.buff.screen_size / 4;
            while (count >= 4) {
                dest[0] = src[0]; dest[1] = src[1];
                dest[2] = src[2]; dest[3] = src[3];
                dest += 4; src += 4; count -= 4;
            }
            while (count--) *dest++ = *src++;
        }
    }
    
    // Draw all visible text objects
    for (int i = 0; i < num_text_objects; i++) {
        text_object *obj = &text_objects[i];
        if (obj->visible) {
            video_draw_string_colored(obj->text, obj->x, obj->y, obj->color, BACK_COLOR);
            obj->dirty = false;
        }
    }
    
    // Single DMA transfer for entire frame
    if (use_dma) {
        video_dma();
    }
    
    frame_dirty = false;
    frame_count++;
    last_frame_time = timer_get_ticks() / 1000 - ms_start;
}

// Get frame timing info
u32 video_get_frame_time() {
    return (u32)last_frame_time;
}

u32 video_get_frame_count() {
    return frame_count;
}

// Force next frame to redraw (useful for animations)
void video_mark_dirty() {
    frame_dirty = true;
}




// CLEAN video_set_resolution without hardcoded demo
void video_set_resolution(u32 xres, u32 yres, u32 bpp) {
    fb_req.res.tag.id = RPI_FIRMWARE_FRAMEBUFFER_SET_PHYSICAL_WIDTH_HEIGHT;
    fb_req.res.tag.buffer_size = 8;
    fb_req.res.tag.value_length = 8;
    fb_req.res.xres = xres;
    fb_req.res.yres = yres;
    
    fb_req.vres.tag.id = RPI_FIRMWARE_FRAMEBUFFER_SET_VIRTUAL_WIDTH_HEIGHT;
    fb_req.vres.tag.buffer_size = 8;
    fb_req.vres.tag.value_length = 8;
    fb_req.vres.xres = xres;
    fb_req.vres.yres = yres;

    fb_req.depth.tag.id = RPI_FIRMWARE_FRAMEBUFFER_SET_DEPTH;
    fb_req.depth.tag.buffer_size = 4;
    fb_req.depth.tag.value_length = 4;
    fb_req.depth.bpp = bpp;

    fb_req.buff.tag.id = RPI_FIRMWARE_FRAMEBUFFER_ALLOCATE;
    fb_req.buff.tag.buffer_size = 8;
    fb_req.buff.tag.value_length = 4;
    fb_req.buff.base = 16;
    fb_req.buff.screen_size = 0;

    fb_req.pitch.tag.id = RPI_FIRMWARE_FRAMEBUFFER_GET_PITCH;
    fb_req.pitch.tag.buffer_size = 4;
    fb_req.pitch.tag.value_length = 4;
    fb_req.pitch.pitch = 0;

    mailbox_set_palette palette;
    palette.tag.id = RPI_FIRMWARE_FRAMEBUFFER_SET_PALETTE;
    palette.tag.buffer_size = 40;
    palette.tag.value_length = 0;
    palette.offset = 0;
    palette.num_entries = 8;
    palette.entries[0] = 0;
    palette.entries[1] = 0xFFBB5500;
    palette.entries[2] = 0xFFFFFFFF;
    palette.entries[3] = 0xFFFF0000;
    palette.entries[4] = 0xFF00FF00;
    palette.entries[5] = 0xFF0000FF;
    palette.entries[6] = 0x55555555;
    palette.entries[7] = 0xCCCCCCCC;

    mailbox_process((mailbox_tag *)&fb_req, sizeof(fb_req));
    printf("Allocated Buffer: %X - %d - %d\n", fb_req.buff.base, fb_req.buff.screen_size, fb_req.depth.bpp);

    if (bpp == 8) {
        mailbox_process((mailbox_tag *)&palette, sizeof(palette));
    }

    clear_screen_once();
    frame_dirty = true;
}

// Keep your existing optimized functions
void video_draw_pixel(u32 x, u32 y, u32 color) {
    if (x >= fb_req.res.xres || y >= fb_req.res.yres) return;
    
    if (fb_req.depth.bpp == 32) {
        u32 *buff = (u32 *)DRAWBUFFER;
        buff[y * (fb_req.pitch.pitch >> 2) + x] = color;
    } else if (fb_req.depth.bpp == 16) {
        u16 *buff = (u16 *)DRAWBUFFER;
        buff[y * (fb_req.pitch.pitch >> 1) + x] = color & 0xFFFF;
    } else {
        u8 *buff = (u8 *)DRAWBUFFER;
        buff[y * fb_req.pitch.pitch + x] = color & 0xFF;
    }
    frame_dirty = true;
}

// USAGE EXAMPLE FOR  OS:
void demo_usage() {
    // Set resolution normally
    video_set_resolution(800, 600, 32);
    
    // Add various text elements
    u32 title_id = video_add_text("My Operating System v1.0", 10, 10, 0xFFFFFFFF);
    u32 status_id = video_add_text("Status: Running", 10, 40, 0xFF00FF00);
    u32 fps_id = video_add_text("FPS: 0", 500, 70, 0xFFFFFF00);
    
    // Main loop (your OS main loop)
    for (int frame = 0; frame < 1000; frame++) {
        // Update dynamic text
        char fps_text[64];
        sprintf(fps_text, "FPS: %d", 1000 / video_get_frame_time());
        video_update_text(fps_id, fps_text);
        
        char status_text[64];
        sprintf(status_text, "Frame: %d", frame);
        video_update_text(status_id, status_text);
        
        // Move title around for demo
        video_move_text(title_id, 10 + (frame % 600), 10);
        
        // Render only when something changed
        video_render_frame(); // Will skip if nothing dirty
        
        
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
