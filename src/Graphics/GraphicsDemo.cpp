#include "Graphics/GraphicsAPI.hpp"
#include "Graphics/GraphicsDemo.hpp"
#include "Graphics/CGraphics_Interop.hpp"
#include "printf.h"
#include "libcpp/types.h"

using namespace Graphics;

// --- Static UI setup ---
static void setup_static_ui(TextObject& title, TextObject& subtitle,
                            TextObject& panelTitle, TextObject& statusLabel,
                            TextObject& uptimeLabel, TextObject& resourceLabel) {
    title.setText("My Operating System v1.0");
    subtitle.setText("Graphics Demo · C/C++ Kernel");
    panelTitle.setText("Status Panel");

    statusLabel.setText("System Status:");
    uptimeLabel.setText("Uptime Monitor:");
    resourceLabel.setText("Resources:");
}

// --- Drawing the panels and background ---
static void draw_background(Renderer& renderer) {
    u32 bg = Colors::Theme::BACKGROUND;
    renderer.drawBox(0, 0, 800, 600, bg);

    // Outer status panel border
    renderer.drawRectOutline(12, 80, 760, 420, Colors::Theme::BORDER, 2);

    // Inner surface
    renderer.drawBox(20, 100, 740, 380, Colors::Theme::SURFACE);

    // Inner panel outline
    renderer.drawRectOutline(20, 100, 740, 380, Colors::Theme::HOVER, 2);

    // Sub-box for resources on right side
    renderer.drawBox(500, 120, 240, 340, Colors::Utils::makeColor(0xFF, 49, 50, 68));
    renderer.drawRectOutline(500, 120, 240, 340, Colors::Theme::BORDER, 2);
}

// --- Updating texts per frame ---
static void update_dynamic_ui(Renderer& renderer,
                              TextObject& fps, TextObject& frameCounter,
                              TextObject& status, TextObject& warning,
                              TextObject& progressBar, TextObject& infoLine,
                              TextObject& panelTitle,
                              TextObject& cpuUsage, TextObject& memUsage,
                              int frame) {
    char buf[128];

    // FPS
    u32 ft = renderer.getFrameTime();
    u32 computed_fps = (ft == 0) ? 0 : (1000 / ft);
    sprintf(buf, "FPS: %u", computed_fps);
    fps.setText(buf);

    // Frame counter
    sprintf(buf, "Frame: %d", frame);
    frameCounter.setText(buf);

    // Status with semantic colors
    if (frame < 600) {
        status.setText("Status: Running");
        status.setColor(Colors::Status::SUCCESS);
    } else if (frame < 900) {
        status.setText("Status: Degraded — testing");
        status.setColor(Colors::Status::WARNING);
    } else {
        status.setText("Status: Error");
        status.setColor(Colors::Status::DANGER);
    }

    // Warning blinking
    warning.setVisible(((frame / 45) % 2) == 0);

    // Progress bar
    const int progress_width_chars = 40;
    static char progress_text[128];
    int prog = (frame <= 500) ? (frame * progress_width_chars / 500) : progress_width_chars;
    for (int i = 0; i < progress_width_chars; ++i)
        progress_text[i] = (i < prog) ? '=' : ' ';
    progress_text[progress_width_chars] = '\0';
    sprintf(buf, "[%s] %3d%%", progress_text, (prog * 100) / progress_width_chars);
    progressBar.setText(buf);

    // Info line (uptime)
    u32 uptime_s = frame / ((ft == 0) ? 60 : (1000 / ft));
    sprintf(buf, "Uptime: %us", uptime_s);
    infoLine.setText(buf);

    // Fake resource monitor values
    sprintf(buf, "CPU Usage: %d%%", (frame * 3) % 100);
    cpuUsage.setText(buf);

    sprintf(buf, "Mem Usage: %d%%", (frame * 7) % 100);
    memUsage.setText(buf);

    // Panel title color swap
    if (frame % 200 < 100) {
        panelTitle.setColor(Colors::Theme::PRIMARY);
    } else {
        panelTitle.setColor(Colors::Theme::SECONDARY);
    }
}

// --- Main demo entry ---
void run_graphics_demo() {
    Renderer renderer;
    if (!renderer.init()) return;
    renderer.configure(1366, 768, 32);
    
    // Text objects
    TextObject title("", 20, 16, Colors::Theme::TEXT);
    TextObject subtitle("", 20, 36, Colors::Theme::SUBTEXT);
    TextObject panelTitle("", 28, 100, Colors::Theme::PRIMARY);

    // Section labels
    TextObject statusLabel("", 36, 120, Colors::Theme::TEXT);
    TextObject uptimeLabel("", 36, 220, Colors::Theme::TEXT);
    TextObject resourceLabel("", 520, 130, Colors::Theme::TEXT);

    // Dynamic texts
    TextObject fps("FPS: 0", 680, 20, Colors::Extended::YELLOW);
    TextObject frameCounter("Frame: 0", 20, 60, Colors::Extended::CYAN);
    TextObject status("Status: Running", 36, 140, Colors::Status::SUCCESS);
    TextObject warning("!! DEMO MODE !!", 420, 460, Colors::Status::DANGER);
    TextObject progressBar("", 36, 240, Colors::Theme::PRIMARY);
    TextObject infoLine("Uptime: 0s", 36, 260, Colors::Theme::SUBTEXT);

    // Fake resource monitor texts
    TextObject cpuUsage("CPU Usage: 0%", 520, 160, Colors::Extended::ORANGE);
    TextObject memUsage("Mem Usage: 0%", 520, 180, Colors::Extended::CYAN);

    // Initialize static labels
    setup_static_ui(title, subtitle, panelTitle, statusLabel, uptimeLabel, resourceLabel);

    for (int frame = 0; frame < 1000; frame++) {
        // Draw boxes and panels
        draw_background(renderer);

        // Update dynamic UI
        update_dynamic_ui(renderer, fps, frameCounter, status, warning,
                          progressBar, infoLine, panelTitle,
                          cpuUsage, memUsage, frame);

        // Present frame
        renderer.presentFrame();

        printf("Demo loop frame: %d\n", frame);
    }
}
