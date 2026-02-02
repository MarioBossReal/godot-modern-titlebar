#if TOOLS

#pragma warning disable SYSLIB1054

using Godot;
using System;
using System.Runtime.InteropServices;

namespace ModernTitlebar;

public static class WindowFrameRemover
{
    const int GWL_STYLE = -16;
    const int GWL_WNDPROC = -4;

    const uint WM_NCCALCSIZE = 0x0083;

    const long WS_CAPTION    = 0x00C00000L;
    const long WS_THICKFRAME = 0x00040000L;

    const uint SWP_NOMOVE       = 0x0002;
    const uint SWP_NOSIZE       = 0x0001;
    const uint SWP_NOZORDER     = 0x0004;
    const uint SWP_FRAMECHANGED = 0x0020;

    const int DWMWA_WINDOW_CORNER_PREFERENCE = 33;
    const int DWMWCP_DEFAULT = 0;
    const int DWMWCP_ROUND = 2;


    const uint WM_NCHITTEST = 0x0084;

    const nint HTCLIENT = 1;
    const nint HTLEFT = 10;
    const nint HTRIGHT = 11;
    const nint HTTOP = 12;
    const nint HTTOPLEFT = 13;
    const nint HTTOPRIGHT = 14;
    const nint HTBOTTOM = 15;
    const nint HTBOTTOMLEFT = 16;
    const nint HTBOTTOMRIGHT = 17;

    [DllImport("user32.dll")]
    static extern bool GetWindowRect(nint hWnd, out RECT lpRect);

    [StructLayout(LayoutKind.Sequential)]
    struct RECT { public int left, top, right, bottom; }

    delegate nint WndProc(nint hWnd, uint msg, nint wParam, nint lParam);

    [DllImport("user32.dll", EntryPoint = "GetWindowLongPtrW")]
    static extern nint GetWindowLongPtr(nint hWnd, int nIndex);

    [DllImport("user32.dll", EntryPoint = "SetWindowLongPtrW")]
    static extern nint SetWindowLongPtr(nint hWnd, int nIndex, nint dwNewLong);

    [DllImport("user32.dll")]
    static extern nint CallWindowProc(nint lpPrevWndFunc, nint hWnd, uint msg, nint wParam, nint lParam);

    [DllImport("user32.dll", SetLastError = true)]
    static extern bool SetWindowPos(nint hWnd, nint hWndInsertAfter, int X, int Y, int cx, int cy, uint uFlags);

    [DllImport("dwmapi.dll")]
    static extern int DwmSetWindowAttribute(nint hwnd, int attr, ref int attrValue, int attrSize);

    static IntPtr _hwnd;
    static nint _oldProc;
    static long _oldStyle;
    static bool _applied;


    static WndProc _proc;

    /// <summary>
    /// <para>Apply changes to the editor window through DWM and user32.</para>
    /// <para>Removes the window caption whilst preserving windows styling (round corners, borders).</para>
    /// </summary>
    public static void Apply()
    {
        if (!OperatingSystem.IsWindows())
            return;

        if (_applied)
            return;

        _hwnd = (IntPtr)DisplayServer.WindowGetNativeHandle(DisplayServer.HandleType.WindowHandle, 0);
        if (_hwnd == IntPtr.Zero)
            return;

        _oldStyle = GetWindowLongPtr(_hwnd, GWL_STYLE);

        var style = _oldStyle;
        style &= ~WS_CAPTION;
        style |= WS_THICKFRAME;
        SetWindowLongPtr(_hwnd, GWL_STYLE, (nint)style);

        int corner = DWMWCP_ROUND;
        _ = DwmSetWindowAttribute(_hwnd, DWMWA_WINDOW_CORNER_PREFERENCE, ref corner, sizeof(int));

        _proc = WindowProc;
        var newPtr = Marshal.GetFunctionPointerForDelegate(_proc);
        _oldProc = SetWindowLongPtr(_hwnd, GWL_WNDPROC, newPtr);

        SetWindowPos(_hwnd, 0, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);

        _applied = true;
    }

    /// <summary>
    /// Revert changes made to the editor window.
    /// </summary>
    public static void Revert()
    {
        if (!OperatingSystem.IsWindows())
            return;

        if (!_applied)
            return;

        if (_hwnd != IntPtr.Zero)
        {
            if (_oldProc != 0)
            {
                SetWindowLongPtr(_hwnd, GWL_WNDPROC, _oldProc);
                _oldProc = 0;
            }


            if (_oldStyle != 0)
                SetWindowLongPtr(_hwnd, GWL_STYLE, (nint)_oldStyle);

            var corner = DWMWCP_DEFAULT;
            _ = DwmSetWindowAttribute(_hwnd, DWMWA_WINDOW_CORNER_PREFERENCE, ref corner, sizeof(int));

            SetWindowPos(_hwnd, 0, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
        }

        _proc = null;
        _hwnd = 0;
        _oldStyle = 0;
        _applied = false;
    }

    static nint WindowProc(nint hWnd, uint msg, nint wParam, nint lParam)
    {
        // Remove non-client area
        if (msg == WM_NCCALCSIZE && wParam != 0 && lParam != 0)
        {
            return 0;
        }

        // Re-implement hit testing for window resizing
        if (msg == WM_NCHITTEST)
        {
            var def = CallWindowProc(_oldProc, hWnd, msg, wParam, lParam);
            if (def != HTCLIENT)
                return def;

            const int border = 8;

            var x = (short)((long)lParam & 0xFFFF);
            var y = (short)(((long)lParam >> 16) & 0xFFFF);

            if (GetWindowRect(hWnd, out RECT wr))
            {
                bool left = x >= wr.left && x < wr.left + border;
                bool right = x < wr.right && x >= wr.right - border;
                bool top = y >= wr.top && y < wr.top + border;
                bool bottom = y < wr.bottom && y >= wr.bottom - border;

                if (top && left) return HTTOPLEFT;
                if (top && right) return HTTOPRIGHT;
                if (bottom && left) return HTBOTTOMLEFT;
                if (bottom && right) return HTBOTTOMRIGHT;
                if (left) return HTLEFT;
                if (right) return HTRIGHT;
                if (top) return HTTOP;
                if (bottom) return HTBOTTOM;
            }

            return HTCLIENT;
        }

        return CallWindowProc(_oldProc, hWnd, msg, wParam, lParam);
    }
}

#pragma warning disable SYSLIB1054

#endif