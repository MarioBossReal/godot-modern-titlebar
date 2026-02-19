#pragma once

#include <godot_cpp/variant/color.hpp>

namespace mtb::win
{
    void apply_titlebar_colors(const godot::Color& caption, const godot::Color& text, bool skip_main_window = false);
    void revert_titlebar_colors();
}