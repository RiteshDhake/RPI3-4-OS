#pragma once

#include "common.h"
#include "dma.h"
#include "mailbox.h"
#include "mm.h"

// void demo_usage();

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


typedef struct  {
    mailbox_tag tag;
    u32 offset;
    u32 num_entries;
    u32 entries[8];
} mailbox_set_palette;



extern mailbox_fb_request fb_req;
extern dma_channel *dma;
extern u8 *vid_buffer;
extern u32 *bg32_buffer;
extern u32 *bg8_buffer;
extern bool use_dma;
extern bool screen_initialized;





#define TEXT_COLOR 0xFFFFFFFF
#define BACK_COLOR 0xFF1E1E2E
#define MB (1024 * 1024)

//hack for not having an allocate function yet...
#define UNCACHED_MEM_BASE 0x40000000
#define BG32_MEM_LOCATION (GRAPH_START_MEMORY + (10 * MB))//14MB
#define BG8_MEM_LOCATION (BG32_MEM_LOCATION + (10 * MB))
#define VB_MEM_LOCATION (BG8_MEM_LOCATION + (4 * MB))//28MB
// 14MB->28MB graphics region

#define BUS_ADDR(x) (((u64)x | 0x40000000) & ~0xC0000000)
#define FRAMEBUFFER ((volatile u8 *)BUS_ADDR(fb_req.buff.base))
#define DMABUFFER ((volatile u8 *)vid_buffer)
#define DRAWBUFFER (use_dma ? DMABUFFER : FRAMEBUFFER)





void video_set_resolution(u32 xres, u32 yres, u32 bpp);

void video_draw_pixel(u32 x, u32 y, u32 color);
void video_init();
void video_set_dma(bool b);
void video_dma();
void do_dma(void *dest, void *src, u32 total);
void video_draw_box(u32 x,u32 y,u32 w,u32 h,u32 color);
void video_draw_rect_outline(u32 x, u32 y, u32 w, u32 h,
                             u32 color, u32 thickness);
