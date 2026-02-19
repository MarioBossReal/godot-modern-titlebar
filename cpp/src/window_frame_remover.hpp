#pragma once

#include <godot_cpp/classes/display_server.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include <windows.h>
#include <dwmapi.h>

#include <cstdint>
#include <cmath>
#include <windowsx.h>

namespace mtb::win 
{
    void apply_window_frame_remover();
    void revert_window_frame_remover();
}