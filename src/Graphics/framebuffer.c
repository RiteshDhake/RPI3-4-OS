#include "mailbox.h"
#include "printf.h"
#include "timer.h"
#include "Graphics/framebuffer.h"
#include "dma.h"
#include "mm.h"
#include <stddef.h>
#include "Graphics/font.h"
#include "Graphics/compositor.h"


mailbox_fb_request fb_req;
dma_channel *dma = NULL;
u8 *vid_buffer = NULL;
u32 *bg32_buffer = NULL;
u32 *bg8_buffer = NULL;
bool use_dma = true;
bool screen_initialized = false;


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
    // num_text_objects = 0;
    frame_dirty = true;
}


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
    printf("Doing DMA transfer");
}

void video_dma() {
    do_dma(FRAMEBUFFER, DMABUFFER, fb_req.buff.screen_size);
}

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


void video_draw_box(u32 x,u32 y,u32 w,u32 h,u32 color){
    u32 *buff = (u32 *)DRAWBUFFER;
    u32 pitch = fb_req.pitch.pitch / 4; 
    for(u32 row = 0 ; row <h;row++){
         u32 offset = (y + row) * pitch + x; 
        for(u32 col = 0 ; col < w ; col++){
            buff[offset + col] = color;
        }
    }
    frame_dirty = 1; // mark for redraw
}

void video_draw_rect_outline(uint32_t x, uint32_t y, uint32_t w, uint32_t h, 
                             uint32_t color, uint32_t thickness) {
    // Top
    video_draw_box(x, y, w, thickness, color);
    // Bottom
    video_draw_box(x, y + h - thickness, w, thickness, color);
    // Left
    video_draw_box(x, y, thickness, h, color);
    // Right
    video_draw_box(x + w - thickness, y, thickness, h, color);
}
