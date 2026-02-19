#include "window_context_menu.hpp"

using namespace godot;

namespace mtb::win 
{
    static constexpr UINT WM_NULL_MSG = 0x0000;

    static HWND get_main_editor_hwnd() {
        auto ds = DisplayServer::get_singleton();
        return (HWND)(intptr_t)ds->window_get_native_handle(DisplayServer::HandleType::WINDOW_HANDLE, 0);
    }

    void show_system_menu_for_main_editor_window() {
        HWND hwnd = get_main_editor_hwnd();
        if (!hwnd) return;

        HMENU hMenu = ::GetSystemMenu(hwnd, FALSE);
        if (!hMenu) return;

        POINT pt{};
        if (!::GetCursorPos(&pt)) return;

        ::SetForegroundWindow(hwnd);

        UINT cmd = ::TrackPopupMenuEx(
            hMenu,
            TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RETURNCMD,
            pt.x,
            pt.y,
            hwnd,
            nullptr
        );

        if (cmd != 0) {
            ::SendMessageW(hwnd, WM_SYSCOMMAND, (WPARAM)cmd, 0);
        }

        ::PostMessageW(hwnd, WM_NULL_MSG, 0, 0);
    }
}