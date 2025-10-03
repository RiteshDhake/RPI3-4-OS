#include "mailbox.h"
#include "printf.h"
#include "timer.h"
#include "Graphics/compositor.h"
#include "dma.h"
#include "mm.h"
#include <stddef.h>



#define MAX_TEXT_OBJECTS 128
#define MAX_TEXT_LENGTH 128


bool frame_dirty = false;
u32 frame_count = 0;
u64 last_frame_time = 0;



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

// static u32 frame_count = 0;
// static u64 last_frame_time = 0;

#define TEXT_COLOR 0xFFFFFFFF
#define BACK_COLOR 0xFF1E1E2E

#define MB (1024 * 1024)

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

// Add text to screen - returns ID for later updates
u32 video_add_text(char *text, u32 x, u32 y, u32 color) {
    if (num_text_objects >= MAX_TEXT_OBJECTS) return 0;
    printf("Adding text '%s' at (%d, %d) color=0x%x\n", text, x, y, color);//debugging 
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
                printf("Updating text");
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



// OPTIMIZED FRAME RENDERING
void video_render_frame() {
    if (!frame_dirty){
        printf("Skipping Frame Render");
        return;
    }  // Skip if nothing changed
    printf("Rendering Frame");
    u64 ms_start = timer_get_ticks() / 1000;
    
    // Clear background (keeping your optimization)
    if (fb_req.depth.bpp == 32) {
        if (use_dma) {
            do_dma((void*)vid_buffer, bg32_buffer, fb_req.buff.screen_size);
            printf("DMA done");
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
            // printf("Text object is Rendering");
        }
    }
    
    // Single DMA transfer for entire frame
    if (use_dma) {
        video_dma();
        printf("Video Dma is called ");
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