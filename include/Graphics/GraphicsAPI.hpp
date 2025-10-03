#ifndef GRAPHICSAPI_HPP
#define GRAPHICSAPI_HPP

#include "libcpp/types.h"

namespace Graphics {

    class Renderer {
    private:
        bool initialized = false;
        bool use_dma = true;
        u32 currentWidth = 0;
        u32 currentHeight = 0;
        u32 currentBpp = 0;

        void cleanup();

    public:
        Renderer() = default;
        ~Renderer() { cleanup(); }

        bool init();
        void configure(u32 w, u32 h, u32 bpp);
        void presentFrame();
        void enableDma(bool enable);
        void markDirty();

        // Performance monitoring
        u32 getFrameTime() const;
        u32 getFrameCount() const;

        // Query methods
        bool isInitialized() const { return initialized; }
        bool isDMAEnabled() const { return use_dma; }

        //graphics
        // Shape drawing
        void drawBox(u32 x, u32 y, u32 w, u32 h, u32 color); // wraps video_draw_box
        void drawRectOutline(u32 x, u32 y, u32 w, u32 h, u32 color, u32 thickness = 2);

    };

    class TextObject {
    private:
        u32 id = 0;
        const char* currentText = nullptr;
        u32 x = 0, y = 0;
        u32 color = 0xFFFFFFFF;
        bool visible = true;
        bool valid = false;

    public:
        // Constructor
        TextObject(const char* text, u32 x, u32 y, u32 color = 0xFFFFFFFF);

        ~TextObject();

        // Move semantics (allow moving but not copying)
        TextObject(TextObject&& other) noexcept;
        TextObject& operator=(TextObject&& other) noexcept;

        // Delete copy operations
        TextObject(const TextObject&) = delete;
        TextObject& operator=(const TextObject&) = delete;

        // Text manipulation
        void setText(const char* text);
        void setPosition(u32 x, u32 y);
        void setColor(u32 color);
        void setVisible(bool visible);

        // Query methods
        const char* getText() const { return currentText; }
        u32 getX() const { return x; }
        u32 getY() const { return y; }
        u32 getColor() const { return color; }
        bool isVisible() const { return visible; }
        bool isValid() const { return valid; }
        u32 getId() const { return id; }
    };

    void clearAllText();

namespace Colors {
    // Basic Colors
    constexpr u32 WHITE = 0xFFFFFFFF;
    constexpr u32 BLACK = 0xFF000000;
    constexpr u32 RED   = 0xFFFF0000;
    constexpr u32 GREEN = 0xFF00FF00;
    constexpr u32 BLUE  = 0xFF0000FF;
    
    // Theme: Dark Purple-Gray Professional Theme
    namespace Theme {
            // Primary Colors
            constexpr u32 BACKGROUND    = 0xFF1E1E2E;  // Rich dark purple-gray
            constexpr u32 SURFACE       = 0xFF313244;  // Elevated dark surface
            constexpr u32 TEXT          = 0xFFCDD6F4;  // Cool light blue-white
            constexpr u32 SUBTEXT       = 0xFFA6ADC8;  // Muted blue-gray
            
            // Accent Colors
            constexpr u32 PRIMARY       = 0xFF89B4FA;  // Soft blue
            constexpr u32 SECONDARY     = 0xFFB4BEFE;  // Light lavender
            constexpr u32 SUCCESS       = 0xFFA6E3A1;  // Mint green
            constexpr u32 WARNING       = 0xFFF9E2AF;  // Warm yellow
            constexpr u32 ERROR         = 0xFFF38BA8;  // Soft red
            
            // UI Elements
            constexpr u32 BORDER        = 0xFF45475A;  // Subtle gray
            constexpr u32 SELECTION     = 0xFF585B70;  // Medium gray
            constexpr u32 HOVER         = 0xFF6C7086;  // Light gray
            constexpr u32 BUTTON        = 0xFF89B4FA;  // Primary blue (same as PRIMARY)
            constexpr u32 INPUT_BG      = 0xFF181825;  // Darker than main background
        }
        
        // Additional Common Colors
        namespace Extended {
            // Grayscale variants
            constexpr u32 GRAY_100      = 0xFFF5F5F5;  // Very light gray
            constexpr u32 GRAY_200      = 0xFFE5E5E5;  // Light gray
            constexpr u32 GRAY_300      = 0xFFD4D4D4;  // Medium light gray
            constexpr u32 GRAY_400      = 0xFFA3A3A3;  // Medium gray
            constexpr u32 GRAY_500      = 0xFF737373;  // Dark medium gray
            constexpr u32 GRAY_600      = 0xFF525252;  // Dark gray
            constexpr u32 GRAY_700      = 0xFF404040;  // Very dark gray
            constexpr u32 GRAY_800      = 0xFF262626;  // Almost black
            constexpr u32 GRAY_900      = 0xFF171717;  // Nearly black
            
            // Vibrant Colors
            constexpr u32 CYAN          = 0xFF00FFFF;  // Bright cyan
            constexpr u32 MAGENTA       = 0xFFFF00FF;  // Bright magenta
            constexpr u32 YELLOW        = 0xFFFFFF00;  // Bright yellow
            constexpr u32 ORANGE        = 0xFFFF8000;  // Orange
            constexpr u32 PURPLE        = 0xFF8000FF;  // Purple
            constexpr u32 LIME          = 0xFF80FF00;  // Lime green
            constexpr u32 PINK          = 0xFFFF80C0;  // Pink
            
            // Pastel Colors
            constexpr u32 PASTEL_BLUE   = 0xFFB3D9FF;  // Light blue
            constexpr u32 PASTEL_GREEN  = 0xFFB3FFB3;  // Light green
            constexpr u32 PASTEL_YELLOW = 0xFFFFFFB3;  // Light yellow
            constexpr u32 PASTEL_PINK   = 0xFFFFB3E6;  // Light pink
            constexpr u32 PASTEL_PURPLE = 0xFFE6B3FF;  // Light purple
            
            // Dark Theme Variants
            constexpr u32 DARK_RED      = 0xFF8B0000;  // Dark red
            constexpr u32 DARK_GREEN    = 0xFF006400;  // Dark green
            constexpr u32 DARK_BLUE     = 0xFF00008B;  // Dark blue
            constexpr u32 DARK_YELLOW   = 0xFF8B8000;  // Dark yellow/olive
            constexpr u32 DARK_CYAN     = 0xFF008B8B;  // Dark cyan
            constexpr u32 DARK_MAGENTA  = 0xFF8B008B;  // Dark magenta
        }
        
        // Status/Semantic Colors
        namespace Status {
            constexpr u32 INFO          = 0xFF17A2B8;  // Bootstrap info blue
            constexpr u32 SUCCESS       = 0xFF28A745;  // Bootstrap success green
            constexpr u32 WARNING       = 0xFFFFC107;  // Bootstrap warning yellow
            constexpr u32 DANGER        = 0xFFDC3545;  // Bootstrap danger red
            constexpr u32 LIGHT         = 0xFFF8F9FA;  // Bootstrap light
            constexpr u32 DARK          = 0xFF343A40;  // Bootstrap dark
        }
        
        // Transparency helpers (can be combined with colors using bitwise operations)
        namespace Alpha {
            constexpr u32 OPAQUE        = 0xFF000000;  // 100% opaque
            constexpr u32 SEMI_OPAQUE   = 0xC0000000;  // 75% opaque
            constexpr u32 HALF          = 0x80000000;  // 50% opaque
            constexpr u32 SEMI_TRANS    = 0x40000000;  // 25% opaque
            constexpr u32 TRANSPARENT   = 0x00000000;  // 0% opaque (fully transparent)
        }
        
        // Utility functions for color manipulation
        namespace Utils {
            // Extract color components
            constexpr u8 getAlpha(u32 color) { return (color >> 24) & 0xFF; }
            constexpr u8 getRed(u32 color)   { return (color >> 16) & 0xFF; }
            constexpr u8 getGreen(u32 color) { return (color >> 8) & 0xFF; }
            constexpr u8 getBlue(u32 color)  { return color & 0xFF; }
            
            // Create color from components
            constexpr u32 makeColor(u8 alpha, u8 red, u8 green, u8 blue) {
                return (static_cast<u32>(alpha) << 24) |
                    (static_cast<u32>(red) << 16) |
                    (static_cast<u32>(green) << 8) |
                    static_cast<u32>(blue);
            }
            
            // Create RGB color (with full alpha)
            constexpr u32 makeRGB(u8 red, u8 green, u8 blue) {
                return makeColor(0xFF, red, green, blue);
            }
            
            // Apply alpha to existing color
            constexpr u32 withAlpha(u32 color, u8 alpha) {
                return (color & 0x00FFFFFF) | (static_cast<u32>(alpha) << 24);
            }
        }
    }

} // namespace Graphics

#endif // GRAPHICSAPI_HPP
