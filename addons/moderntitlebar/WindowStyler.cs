#if TOOLS

#pragma warning disable SYSLIB1054

using Godot;
using System;
using System.Runtime.InteropServices;

namespace ModernTitlebar;

public static class WindowStyler
{
    const int DWMWA_CAPTION_COLOR = 35;
    const int DWMWA_TEXT_COLOR = 36;
    const uint DWMWA_COLOR_DEFAULT = 0xFFFFFFFF;

    [DllImport("dwmapi.dll")]
    static extern int DwmSetWindowAttribute(
        IntPtr hwnd, 
        int dwAttribute, 
        ref uint pvAttribute, 
        int cbAttribute
    );

    static uint ToSystemColor(Color c)
    {
        var r = (byte)Math.Clamp((int)MathF.Round(c.R * 255f), 0, 255);
        var g = (byte)Math.Clamp((int)MathF.Round(c.G * 255f), 0, 255);
        var b = (byte)Math.Clamp((int)MathF.Round(c.B * 255f), 0, 255);

        return (uint)(r | (g << 8) | (b << 16));
    }

    /// <summary>
    /// <para>Sets the colours used by the native titlebars of the editor's windows.</para>
    /// <para>Used to colour match subwindows without much fuss.</para>
    /// </summary>
    /// <param name="caption"></param>
    /// <param name="text"></param>
    public static void ApplyTitlebarColors(Color caption, Color text, bool skipMainWindow = false)
    {
        if (!OperatingSystem.IsWindows())
            return;

        var windows = DisplayServer.GetWindowList();

        uint captionColor = ToSystemColor(caption);
        uint textColor = ToSystemColor(text);

        foreach (var window in windows)
        {
            if (window == 0 && skipMainWindow)
                continue;

            var hwnd = (IntPtr)DisplayServer.WindowGetNativeHandle(DisplayServer.HandleType.WindowHandle, window);
            if (hwnd == IntPtr.Zero)
                continue;
            
            var popup = DisplayServer.WindowGetFlag(DisplayServer.WindowFlags.Popup, window);
            if (popup)
                continue;

            var nofocus = DisplayServer.WindowGetFlag(DisplayServer.WindowFlags.NoFocus, window);
            if (nofocus)
                continue;

            var borderless = DisplayServer.WindowGetFlag(DisplayServer.WindowFlags.Borderless, window);
            if (borderless)
                continue;
            
            _ = DwmSetWindowAttribute(hwnd, DWMWA_CAPTION_COLOR, ref captionColor, sizeof(uint));
            _ = DwmSetWindowAttribute(hwnd, DWMWA_TEXT_COLOR, ref textColor, sizeof(uint));
        }
    }

    public static void RevertTitlebarColors()
    {
        if (!OperatingSystem.IsWindows())
            return;

        var windows = DisplayServer.GetWindowList();

        uint captionColor = DWMWA_COLOR_DEFAULT;

        foreach (var window in windows)
        {
            var hwnd = (IntPtr)DisplayServer.WindowGetNativeHandle(DisplayServer.HandleType.WindowHandle, window);

            if (hwnd == IntPtr.Zero)
                continue;
            
            var popup = DisplayServer.WindowGetFlag(DisplayServer.WindowFlags.Popup, window);
            if (popup)
                continue;
            
            _ = DwmSetWindowAttribute(hwnd, DWMWA_CAPTION_COLOR, ref captionColor, sizeof(uint));
            _ = DwmSetWindowAttribute(hwnd, DWMWA_TEXT_COLOR, ref captionColor, sizeof(uint));
        }
    }
}

#pragma warning restore SYSLIB1054


#endif
