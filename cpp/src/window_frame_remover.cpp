#include "window_frame_remover.hpp"

using namespace godot;

namespace mtb::win
{
    static constexpr int GWL_WNDPROC = -4;
    static constexpr int DWMWA_WINDOW_CORNER_PREFERENCE = 33;
    static constexpr int DWMWCP_DEFAULT = 0;
    static constexpr int DWMWCP_ROUND = 2;

    static HWND     g_hwnd = nullptr;
    static WNDPROC  g_original_proc = nullptr;
    static LONG_PTR g_original_style = 0;
    static int      g_original_corner_pref = DWMWCP_DEFAULT;
    static bool     g_applied = false;

    static HWND get_main_editor_hwnd() {
        auto ds = DisplayServer::get_singleton();
        return (HWND)(intptr_t)ds->window_get_native_handle(DisplayServer::HandleType::WINDOW_HANDLE, 0);
    }

    static float get_screen_scale_96dpi() {
        auto ds = DisplayServer::get_singleton();
        return (float)ds->screen_get_dpi() / 96.0f;
    }

    static LRESULT CALLBACK ModernTitlebarWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {

        if (msg == WM_NCCALCSIZE && wParam != 0 && lParam != 0) {
            NCCALCSIZE_PARAMS* p = reinterpret_cast<NCCALCSIZE_PARAMS*>(lParam);

            if (::IsZoomed(hWnd)) {
                const float scale = get_screen_scale_96dpi();
                const int padding = (int)std::lround(8.0f * scale);

                p->rgrc[0].left += padding;
                p->rgrc[0].top += padding;
                p->rgrc[0].right -= padding;
                p->rgrc[0].bottom -= padding;
            }

            return 0;
        }

        if (msg == WM_NCHITTEST) {

            if (!g_original_proc) {
                return HTCLIENT;
            }

            const LRESULT def = ::CallWindowProcW(g_original_proc, hWnd, msg, wParam, lParam);
            if (def != HTCLIENT) {
                return def;
            }

            const int border = 8;

            const int x = GET_X_LPARAM(lParam);
            const int y = GET_Y_LPARAM(lParam);

            RECT wr{};

            if (::GetWindowRect(hWnd, &wr)) {
                const bool left = (x >= wr.left) && (x < wr.left + border);
                const bool right = (x < wr.right) && (x >= wr.right - border);
                const bool top = (y >= wr.top) && (y < wr.top + border);
                const bool bottom = (y < wr.bottom) && (y >= wr.bottom - border);

                if (top && left)     return HTTOPLEFT;
                if (top && right)    return HTTOPRIGHT;
                if (bottom && left)  return HTBOTTOMLEFT;
                if (bottom && right) return HTBOTTOMRIGHT;
                if (left)            return HTLEFT;
                if (right)           return HTRIGHT;
                if (top)             return HTTOP;
                if (bottom)          return HTBOTTOM;
            }

            return HTCLIENT;
        }

        if (!g_original_proc) {
            return ::DefWindowProcW(hWnd, msg, wParam, lParam);
        }
        return ::CallWindowProcW(g_original_proc, hWnd, msg, wParam, lParam);
    }

    void apply_window_frame_remover() {
        if (g_applied) {
            return;
        }

        g_hwnd = get_main_editor_hwnd();
        if (!g_hwnd) {
            return;
        }

        g_original_style = ::GetWindowLongPtrW(g_hwnd, GWL_STYLE);
        g_original_proc = (WNDPROC)::GetWindowLongPtrW(g_hwnd, GWL_WNDPROC);

        g_original_corner_pref = DWMWCP_DEFAULT;
        (void)::DwmGetWindowAttribute(
            g_hwnd,
            DWMWA_WINDOW_CORNER_PREFERENCE,
            &g_original_corner_pref,
            sizeof(g_original_corner_pref)
        );

        if (!g_original_proc) {
            g_hwnd = nullptr;
            g_original_style = 0;
            g_original_corner_pref = DWMWCP_DEFAULT;
            return;
        }

        LONG_PTR style = g_original_style;
        style &= ~WS_CAPTION;
        style |= WS_THICKFRAME;
        ::SetWindowLongPtrW(g_hwnd, GWL_STYLE, style);

        int corner = DWMWCP_ROUND;
        (void)::DwmSetWindowAttribute(g_hwnd, DWMWA_WINDOW_CORNER_PREFERENCE, &corner, sizeof(corner));

        ::SetWindowLongPtrW(g_hwnd, GWL_WNDPROC, (LONG_PTR)&ModernTitlebarWndProc);

        ::SetWindowPos(
            g_hwnd,
            nullptr,
            0, 0, 0, 0,
            SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED
        );

        g_applied = true;
    }

    void revert_window_frame_remover() {
        if (!g_applied) {
            return;
        }

        if (!g_hwnd) {
            g_hwnd = get_main_editor_hwnd();
        }
        if (!g_hwnd) {
            g_original_proc = nullptr;
            g_original_style = 0;
            g_original_corner_pref = DWMWCP_DEFAULT;
            g_applied = false;
            return;
        }

        if (g_original_proc) {
            ::SetWindowLongPtrW(g_hwnd, GWL_WNDPROC, (LONG_PTR)g_original_proc);
        }

        if (g_original_style != 0) {
            ::SetWindowLongPtrW(g_hwnd, GWL_STYLE, g_original_style);
        }

        int corner = g_original_corner_pref;
        (void)::DwmSetWindowAttribute(g_hwnd, DWMWA_WINDOW_CORNER_PREFERENCE, &corner, sizeof(corner));

        ::SetWindowPos(
            g_hwnd,
            nullptr,
            0, 0, 0, 0,
            SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED
        );

        g_hwnd = nullptr;
        g_original_proc = nullptr;
        g_original_style = 0;
        g_original_corner_pref = DWMWCP_DEFAULT;
        g_applied = false;
    }
}