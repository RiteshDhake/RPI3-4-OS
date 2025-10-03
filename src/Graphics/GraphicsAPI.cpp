#include "Graphics/GraphicsAPI.hpp"
#include "Graphics/CGraphics_Interop.hpp"
#include "libcpp/assert.h"
#include "libcpp/types.h"

using namespace Graphics;

#define THROW_ERROR(msg) panic(msg, __FILE__, __LINE__)

// ---------------- Renderer -----------------

void Renderer::cleanup() {
    initialized = false;
    use_dma = true;
    currentWidth = 0;
    currentHeight = 0;
    currentBpp = 0;
}

bool Renderer::init() {
    if (initialized) return true;

    video_init();
    initialized = true;
    return true;
}

void Renderer::configure(u32 w, u32 h, u32 bpp) {
    if (!initialized)
        THROW_ERROR("Renderer not initialized");
    if (w < 320 || h < 200 || (bpp != 8 && bpp != 16 && bpp != 32))
        THROW_ERROR("Unsupported resolution or bpp");

    video_set_resolution(w, h, bpp);
    currentWidth = w;
    currentHeight = h;
    currentBpp = bpp;
}

void Renderer::presentFrame() {
    if (!initialized)
        THROW_ERROR("Renderer not initialized");
    video_render_frame();
}

void Renderer::enableDma(bool enable) {
    use_dma = enable;
    video_set_dma(enable);
}

void Renderer::markDirty() {
    video_mark_dirty();
}

u32 Renderer::getFrameTime() const {
    return video_get_frame_time();
}

u32 Renderer::getFrameCount() const {
    return video_get_frame_count();
}

void Renderer::drawBox(u32 x, u32 y, u32 w, u32 h, u32 color) {
    if (!initialized) THROW_ERROR("Renderer not initialized");
    video_draw_box(x, y, w, h, color);
}

void Renderer::drawRectOutline(u32 x, u32 y, u32 w, u32 h, u32 color, u32 thickness) {
    if (!initialized) THROW_ERROR("Renderer not initialized");
    video_draw_rect_outline(x, y, w, h, color, thickness);
}


// ---------------- TextObject -----------------

TextObject::TextObject(const char* text, u32 x, u32 y, u32 color)
    : currentText(text), x(x), y(y), color(color), visible(true), valid(false)
{
    id = video_add_text(const_cast<char*>(currentText), x, y, color);
    if (id == 0) {
        THROW_ERROR("Failed to create text object (max reached?)");
    }
    valid = true;
}

TextObject::~TextObject() {
    if (valid) {
        video_remove_text(id);
        valid = false;
    }
}

// Move constructor
TextObject::TextObject(TextObject&& other) noexcept {
    // Manually steal resources
    id          = other.id;
    currentText = other.currentText;
    x           = other.x;
    y           = other.y;
    color       = other.color;
    visible     = other.visible;
    valid       = other.valid;

    // Reset source
    other.id = 0;
    other.currentText = nullptr;
    other.valid = false;
}

// Move assignment
TextObject& TextObject::operator=(TextObject&& other) noexcept {
    if (this != &other) {
        // Clean up current
        if (valid) {
            video_remove_text(id);
        }

        // Steal resources
        id          = other.id;
        currentText = other.currentText;
        x           = other.x;
        y           = other.y;
        color       = other.color;
        visible     = other.visible;
        valid       = other.valid;

        // Reset source
        other.id = 0;
        other.currentText = nullptr;
        other.valid = false;
    }
    return *this;
}

void TextObject::setText(const char* text) {
    if (!valid) THROW_ERROR("Invalid TextObject");
    currentText = text;
    video_update_text(id, const_cast<char*>(currentText));
}

void TextObject::setPosition(u32 newX, u32 newY) {
    if (!valid) THROW_ERROR("Invalid TextObject");
    x = newX;
    y = newY;
    video_move_text(id, x, y);
}

void TextObject::setColor(u32 newColor) {
    if (!valid) THROW_ERROR("Invalid TextObject");
    color = newColor;
    // Remove & re-add to apply new color (API limitation)
    video_remove_text(id);
    id = video_add_text(const_cast<char*>(currentText), x, y, color);
    if (id == 0) THROW_ERROR("Failed to reapply color");
}

void TextObject::setVisible(bool vis) {
    if (!valid) THROW_ERROR("Invalid TextObject");
    visible = vis;
    video_set_text_visible(id, vis);
}

// ---------------- Global helpers -----------------

void Graphics::clearAllText() {
    video_clear_all_text();
}
