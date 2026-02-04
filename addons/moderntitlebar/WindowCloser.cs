#if TOOLS

#pragma warning disable SYSLIB1054

using System;
using System.Runtime.InteropServices;
using Godot;

namespace ModernTitlebar;

public static class WindowCloser
{
    const uint WM_CLOSE = 0x0010;

    [DllImport("user32.dll")]
    static extern bool PostMessage(nint hWnd, uint Msg, nint wParam, nint lParam);

    /// <summary>
    /// <para>Replicates the request sent by clicking the native window close button.</para>
    /// <para>This allows the editor to prompt for unsaved scenes/assets rather than forcefully shutting down and losing unsaved work.</para>
    /// </summary>
    public static void RequestCloseMainEditorWindow()
    {
        var hwnd = (nint)DisplayServer.WindowGetNativeHandle(DisplayServer.HandleType.WindowHandle, 0);
        if (hwnd == 0)
            return;

        PostMessage(hwnd, WM_CLOSE, 0, 0);
    }
}

#pragma warning restore SYSLIB1054

#endif