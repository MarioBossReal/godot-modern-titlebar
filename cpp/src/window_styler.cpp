#include "window_styler.hpp"

#include <godot_cpp/classes/display_server.hpp>
#include <godot_cpp/variant/utility_functions.hpp>


#include <windows.h>
#include <dwmapi.h>

#include <cstdint>
#include <algorithm>
#include <cmath>

using namespace godot;

namespace mtb::win 
{
    static constexpr int DWMWA_CAPTION_COLOR = 35;
    static constexpr int DWMWA_TEXT_COLOR = 36;

    static inline uint32_t clamp_u8_from_01(float v01) {
        float v = std::round(v01 * 255.0f);
        if (v < 0.0f) v = 0.0f;
        if (v > 255.0f) v = 255.0f;
        return static_cast<uint32_t>(v);
    }

    static uint32_t to_system_color(const Color& c) {
        uint32_t r = clamp_u8_from_01(c.r);
        uint32_t g = clamp_u8_from_01(c.g);
        uint32_t b = clamp_u8_from_01(c.b);
        return (r) | (g << 8) | (b << 16);
    }

    static HWND hwnd_for_window_id(int64_t window_id) {
        auto ds = DisplayServer::get_singleton();
        return (HWND)(intptr_t)ds->window_get_native_handle(DisplayServer::HandleType::WINDOW_HANDLE, (int)window_id);
    }

    void apply_titlebar_colors(const Color& caption, const Color& text, bool skip_main_window) {
        auto ds = DisplayServer::get_singleton();

        const auto caption_color = to_system_color(caption);
        const auto text_color = to_system_color(text);

        const auto windows = ds->get_window_list();

        for (int i = 0; i < windows.size(); i++) {
            const auto window_id = windows[i];

            if (window_id == 0 && skip_main_window) {
                continue;
            }

            HWND hwnd = hwnd_for_window_id(window_id);
            if (!hwnd) continue;

            if (ds->window_get_flag(DisplayServer::WINDOW_FLAG_POPUP, window_id))       continue;
            if (ds->window_get_flag(DisplayServer::WINDOW_FLAG_NO_FOCUS, window_id))    continue;
            if (ds->window_get_flag(DisplayServer::WINDOW_FLAG_BORDERLESS, window_id)) continue;
            if (ds->window_get_flag(DisplayServer::WINDOW_FLAG_TRANSPARENT, window_id))continue;

            uint32_t cap = caption_color;
            uint32_t txt = text_color;

            (void)::DwmSetWindowAttribute(hwnd, DWMWA_CAPTION_COLOR, &cap, sizeof(cap));
            (void)::DwmSetWindowAttribute(hwnd, DWMWA_TEXT_COLOR, &txt, sizeof(txt));
        }
    }

    void revert_titlebar_colors() {
        auto ds = DisplayServer::get_singleton();

        const PackedInt32Array windows = ds->get_window_list();
        for (int i = 0; i < windows.size(); i++) {
            const auto window_id = windows[i];

            HWND hwnd = hwnd_for_window_id(window_id);
            if (!hwnd) continue;

            if (ds->window_get_flag(DisplayServer::WINDOW_FLAG_POPUP, window_id)) continue;

            uint32_t def = DWMWA_COLOR_DEFAULT;
            (void)::DwmSetWindowAttribute(hwnd, DWMWA_CAPTION_COLOR, &def, sizeof(def));
            (void)::DwmSetWindowAttribute(hwnd, DWMWA_TEXT_COLOR, &def, sizeof(def));
        }
    }

}