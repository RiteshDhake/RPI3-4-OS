#ifndef C_GRAPHICS_INTEROP_H
#define C_GRAPHICS_INTEROP_H


// bridge between CPP and C; Allows CPP to use C functions

extern "C" {
    #include "framebuffer.h"
    #include "font.h" 
    #include "compositor.h"
    #include "printf.h"
}

#endif