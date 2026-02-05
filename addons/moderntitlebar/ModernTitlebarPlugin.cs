#if TOOLS
using Godot;
using System;
using System.IO;

namespace ModernTitlebar;

[Tool]
public partial class ModernTitlebarPlugin : EditorPlugin, ISerializationListener
{
    string AddonRoot { get; set; }

    // NATIVE WINDOW ICONS
    const string CloseIcon = "\uE8BB";
    const string MinimizeIcon = "\uE921";
    const string MaximizeIcon = "\uE922";
    const string RestoreIcon = "\uE923";
    
	// META
	static readonly StringName META_ORIGINAL_PROC = "original_proc";
	static readonly StringName META_ORIGINAL_STYLE= "original_style";

	// THEME OVERRIDES
    static readonly StringName EDITOR = "Editor";
    static readonly StringName PANEL = "panel";
    static readonly StringName FONT = "font";
    static readonly StringName FONT_COLOR = "font_color";
    static readonly StringName FONT_SIZE = "font_size";
    static readonly StringName MARGIN_LEFT = "margin_left";
    static readonly StringName MARGIN_RIGHT = "margin_right";
    static readonly StringName MARGIN_TOP = "margin_top";
    static readonly StringName MARGIN_BOTTOM = "margin_bottom";
    static readonly StringName SEPARATION = "separation";
    static readonly StringName SEPARATOR = "separator";
    static readonly StringName V_SEPARATION = "v_separation";
    static readonly StringName NORMAL = "normal";
    static readonly StringName HOVER = "hover";
    static readonly StringName PRESSED = "pressed";
    static readonly StringName HOVER_PRESSED = "hover_pressed";

	// SIZE / SCALE
    Vector2 WindowButtonSize { get; set; }
    float ScreenScale { get; set; }
	Color BackgroundColor { get; set; }

    // PLUGIN CONTROLS
    MarginContainer ModernTitlebar { get; set; }
	Control MenuBarRoot { get; set; }
	Control RunBarRoot { get; set; }
	Button DragButton { get; set; }
	MarginContainer WindowButtons { get; set; }
	HBoxContainer WindowButtonsHBox { get; set; }
    Button MinimiseButton { get; set; }
    Button MaximiseButton { get; set; }
    Button CloseButton { get; set; }
	MarginContainer IconPadding { get; set; }

	// PLUGIN STYLING
	StyleBoxFlat MenuBarNormalStyle { get; set; }
	StyleBoxFlat MenuBarHoverStyle { get; set; }
	StyleBoxFlat MenuBarSelectedStyle { get; set; }
	StyleBoxFlat PopupPanelStyle { get; set; }
	StyleBoxFlat PopupSeparatorStyle { get; set; }

	// A workaround to the fact that the EditorMainScreen's functionality depends extremely heavily on the buttons' names
	FontFile BlankFont { get; set; }

    // EDITOR CONTROLS
    Window EditorWindow { get; set; }
	Node EditorNode { get; set; }
	VBoxContainer EditorMainVBox { get; set; }
    Panel EditorBaseControl { get; set; }
    Control EditorTitleBar { get; set; }
	MenuBar EditorMenuBar { get; set; }
	Control EditorRunBar { get; set; }
	PanelContainer EditorRunBarPanel { get; set; }
    HBoxContainer EditorMainScreenButtons { get; set; }
	Control EditorSceneTabs { get; set; }
    HBoxContainer EditorSceneTabsHBox { get; set; }

	// EDITOR DEFAULTS
    Control.SizeFlags MenuBarSizeFlagsH;
	Control.SizeFlags RunBarSizeFlagsH;
    Control.SizeFlags MenuBarSizeFlagsV;
    Control.SizeFlags RunBarSizeFlagsV;

    public override void _EnterTree()
	{
		if (!OperatingSystem.IsWindows())
			return;

        AddonRoot = Path.GetDirectoryName(
			GetScript().As<Script>()
			.ResourcePath)
			.Replace("res:\\", "res://")
			.Replace('\\', '/');

        // Windows DPI is 96 at 100% scaling
        ScreenScale = DisplayServer.ScreenGetDpi() / 96f;
		BlankFont = ResourceLoader.Load<FontFile>($"{AddonRoot}/AdobeBlank.ttf");

        // Setup titlebar
        var titleBarPrefab = ResourceLoader.Load<PackedScene>($"{AddonRoot}/modern_titlebar.scn");
		ModernTitlebar = titleBarPrefab.Instantiate<MarginContainer>();
		MenuBarRoot = ModernTitlebar.GetNode("%MenuBarRoot") as Control;
		RunBarRoot = ModernTitlebar.GetNode("%RunBarRoot") as Control;
		DragButton = ModernTitlebar.GetNode("%Drag") as Button;
		DragButton.ButtonDown += OnDragPressed;
		DragButton.GuiInput += OnDragGuiInput;

		// Setup window buttons
		var buttonsPrefab = ResourceLoader.Load<PackedScene>($"{AddonRoot}/window_buttons.scn");
		WindowButtons = buttonsPrefab.Instantiate<MarginContainer>();
		WindowButtonsHBox = WindowButtons.GetNode("%HBoxContainer") as HBoxContainer;
        MinimiseButton = WindowButtons.GetNode("%Minimise") as Button;
		MaximiseButton = WindowButtons.GetNode("%Maximise") as Button;
		CloseButton = WindowButtons.GetNode("%Close") as Button;

		// 45:29 aspect
        WindowButtonSize = new(ScaleInt(45), ScaleInt(29));

        MinimiseButton.CustomMinimumSize = WindowButtonSize;
		MaximiseButton.CustomMinimumSize = WindowButtonSize;
		CloseButton.CustomMinimumSize = WindowButtonSize;

		MinimiseButton.Pressed += OnMinimisePressed;
		MaximiseButton.Pressed += OnMaximisePressed;
		CloseButton.Pressed += OnClosePressed;

		var buttonFontSize = ScaleInt(10);

		MinimiseButton.AddThemeFontSizeOverride(FONT_SIZE, buttonFontSize);
		MaximiseButton.AddThemeFontSizeOverride(FONT_SIZE, buttonFontSize);
		CloseButton.AddThemeFontSizeOverride(FONT_SIZE, buttonFontSize);
		WindowButtonsHBox.AddThemeConstantOverride(SEPARATION, ScaleInt(1));

		// Icon padding
		IconPadding = ModernTitlebar.GetNode("%IconPadding") as MarginContainer;

        // Get handles to editor control nodes
        EditorWindow = GetTree().Root;
		EditorNode = EditorWindow.GetChild(0);
		EditorBaseControl = EditorInterface.Singleton.GetBaseControl() as Panel;
		EditorMainVBox = EditorBaseControl.GetChild(0) as VBoxContainer;
		EditorTitleBar = EditorBaseControl.FindChild("*EditorTitleBar*", true, false) as Control;
		EditorMenuBar = EditorTitleBar.FindChild("*MenuBar*", true, false) as MenuBar;
		EditorRunBar = EditorTitleBar.FindChild("*EditorRunBar*", true, false) as Control;
		EditorRunBarPanel = EditorRunBar.FindChild("*PanelContainer*", true, false) as PanelContainer;
		EditorMainScreenButtons = EditorTitleBar.FindChild("*EditorMainScreenButtons*", true, false) as HBoxContainer;
		EditorSceneTabs = EditorBaseControl.FindChild("*EditorSceneTabs*", true, false) as Control;
		EditorSceneTabsHBox = EditorSceneTabs.FindChild("*HBoxContainer*", true, false) as HBoxContainer;

		// Save default size flags
		MenuBarSizeFlagsH = EditorMenuBar.SizeFlagsHorizontal;
		RunBarSizeFlagsH = EditorRunBar.SizeFlagsHorizontal;
        MenuBarSizeFlagsV = EditorMenuBar.SizeFlagsVertical;
        RunBarSizeFlagsV = EditorRunBar.SizeFlagsVertical;

		// Listen to window size changed
        EditorWindow.SizeChanged += OnWindowSizeChanged;

		// Add plugin controls to editor
		EditorMainVBox.AddChild(ModernTitlebar);
		EditorMainVBox.MoveChild(ModernTitlebar, 0);
		EditorBaseControl.AddChild(WindowButtons);
		EditorBaseControl.MoveChild(WindowButtons, 1);

        BackgroundColor = GetBackgroundColor();

		// Create Plugin Styling
		CreatePluginStyleBoxes();

        // Apply changes to editor controls
        ApplyMainScreenButtonsChanges();
		ApplyEditorRunBarChanges();
		ApplyEditorMenuBarChanges();
		ApplyEditorPopupMenuStyleChanges();

		// Hide default editor titlebar
		EditorTitleBar.Hide();

		// Theme native titlebars of editor subwindows
		ApplyTitlebarColors();

		// Remove native windows titlebar
        WindowFrameRemover.Apply();

		// Update
		OnWindowSizeChanged();

		// Make sure titlebar visible
		var pos = DisplayServer.WindowGetPosition(0);
		pos.Y = Mathf.Max(pos.Y, 0);
		DisplayServer.WindowSetPosition(pos);

		GetTree().NodeAdded += OnSceneTreeNodeAdded;
    }

	public override void _DisablePlugin()
	{
		if (!OperatingSystem.IsWindows())
			return;

		RevertEditorPopupMenuStyleChanges();
        RevertEditorMenuBarChanges();
		RevertMainScreenButtonsChanges();
		RevertEditorRunBarChanges();

		EditorMainVBox.RemoveChild(ModernTitlebar);
		ModernTitlebar.QueueFree();
		EditorBaseControl.RemoveChild(WindowButtons);
		WindowButtons.QueueFree();

		EditorTitleBar.Show();
		WindowStyler.RevertTitlebarColors();
		WindowFrameRemover.Revert();
    }

    public override void _Process(double delta)
    {
		if (!OperatingSystem.IsWindows())
			return;

		// Force apply native titlebar colours for editor subwindows
        ApplyTitlebarColors(true);

		// Force apply panel styling if the theme changes
        var stylebox = EditorRunBarPanel.GetThemeStylebox(PANEL);
		if (stylebox is StyleBoxEmpty empty && empty.ContentMarginTop != 0)
		{
			ApplyEditorRunBarPanelStyling();
		}


		var backgroundColor = GetBackgroundColor();
		if (backgroundColor != BackgroundColor)
		{
			BackgroundColor = backgroundColor;
			ApplyEditorPopupMenuStyleChanges();
		}
    }

	void ApplyTitlebarColors(bool skipMainWindow = false)
	{
        var titleColor = GetBackgroundColor();
        var textColor = GetTextColor();
        WindowStyler.ApplyTitlebarColors(titleColor, textColor, skipMainWindow);
    }

	void OnWindowSizeChanged()
	{
		var max = EditorWindow.Mode == Window.ModeEnum.Maximized;

        var pos = DisplayServer.WindowGetPosition(0);

		var mT = max ? Math.Abs(pos.X) : ScaleInt(-3);
		var mW = max ? Math.Abs(pos.X) : ScaleInt(1);

		var tbLeft = ScaleInt(-3);
		var tbBottom = ScaleInt(-4);

        SetTitlebarMargins(tbLeft, 0, mT, max ? 0 : tbBottom);
        SetWindowButtonMargins(0, mW, mW, 0);

		var iconPad = max ? ScaleInt(Math.Abs(pos.X) + 4) : ScaleInt(8);
		IconPadding.AddThemeConstantOverride(MARGIN_LEFT, iconPad);

		MaximiseButton.Text = max ? RestoreIcon : MaximizeIcon;
    }

	void OnMinimisePressed()
	{
		EditorWindow.Mode = Window.ModeEnum.Minimized;
	}

	void OnMaximisePressed()
	{
		if (EditorWindow.Mode == Window.ModeEnum.Maximized)
            EditorWindow.Mode = Window.ModeEnum.Windowed;
		else
            EditorWindow.Mode = Window.ModeEnum.Maximized;
	}
    void OnDragPressed()
    {
        EditorWindow.StartDrag();
    }

    void OnClosePressed()
	{
		WindowCloser.RequestCloseMainEditorWindow();
    }

	void OnEditorSceneButtonChildEnteredTree(Node child)
	{
		// Support editor plugins that implement main screens that are enabled after this plugin
		if (child is not Button button)
			return;

		button.AddThemeFontSizeOverride(FONT_SIZE, 1);
		button.AddThemeFontOverride(FONT, BlankFont);
		button.IconAlignment = HorizontalAlignment.Center;
	}

	void OnDragGuiInput(InputEvent @event)
	{
        if (@event is not InputEventMouseButton mouse)
            return;

        if (mouse.ButtonIndex != MouseButton.Right)
            return;

        if (!mouse.Pressed)
            return;

        WindowContextMenu.ShowForMainEditorWindow();
    }

	void OnSceneTreeNodeAdded(Node node)
	{
		if (node is PopupMenu popup && !EditorMenuBar.IsAncestorOf(popup))
		{
			var fontSize = ScaleInt(12);
			popup.AddThemeConstantOverride(V_SEPARATION, fontSize);
			popup.AddThemeFontSizeOverride(FONT_SIZE, fontSize);
			popup.AddThemeStyleboxOverride(PANEL, PopupPanelStyle);
			popup.AddThemeStyleboxOverride(SEPARATOR, PopupSeparatorStyle);
		}
	}

	void CreatePluginStyleBoxes()
	{
        MenuBarHoverStyle = EditorMenuBar.GetThemeStylebox(NORMAL, EDITOR).Duplicate() as StyleBoxFlat;

        var radius = ScaleInt(4);

        MenuBarHoverStyle.CornerRadiusBottomLeft = radius;
        MenuBarHoverStyle.CornerRadiusTopLeft = radius;
        MenuBarHoverStyle.CornerRadiusBottomRight = radius;
        MenuBarHoverStyle.CornerRadiusTopRight = radius;
        MenuBarHoverStyle.CornerDetail = 8;
        MenuBarHoverStyle.BorderWidthBottom = 0;
        MenuBarHoverStyle.BorderWidthTop = 0;
        MenuBarHoverStyle.BorderWidthLeft = 0;
        MenuBarHoverStyle.BorderWidthRight = 0;
        MenuBarHoverStyle.DrawCenter = true;
        MenuBarHoverStyle.BgColor = GetPopupPanelColor();

        MenuBarNormalStyle = MenuBarHoverStyle.Duplicate() as StyleBoxFlat;
        MenuBarNormalStyle.DrawCenter = false;

        MenuBarSelectedStyle = MenuBarHoverStyle.Duplicate() as StyleBoxFlat;
        MenuBarSelectedStyle.CornerRadiusBottomLeft = 0;
        MenuBarSelectedStyle.CornerRadiusBottomRight = 0;

        PopupPanelStyle = new StyleBoxFlat();
        var contentMargin = ScaleInt(4);
        PopupPanelStyle.ContentMarginBottom = contentMargin;
        PopupPanelStyle.ContentMarginTop = contentMargin;
        PopupPanelStyle.ContentMarginLeft = contentMargin;
        PopupPanelStyle.ContentMarginRight = contentMargin;
        PopupPanelStyle.BgColor = MenuBarHoverStyle.BgColor;

        PopupSeparatorStyle = new StyleBoxFlat();
        PopupSeparatorStyle.BgColor = Colors.Transparent;
        var separatorColor = GetTextColor();
        separatorColor.A = 0.098f;
        PopupSeparatorStyle.BorderColor = separatorColor;
        PopupSeparatorStyle.BorderWidthTop = ScaleInt(1);
    }

	void ApplyMainScreenButtonsChanges()
	{
		// Move the main screen buttons to the editor scene tabs control and hide their text

		EditorTitleBar.RemoveChild(EditorMainScreenButtons);
		EditorSceneTabsHBox.AddChild(EditorMainScreenButtons);
		EditorSceneTabsHBox.MoveChild(EditorMainScreenButtons, 3);

		foreach (var c in EditorMainScreenButtons.GetChildren())
		{
			if (c is not Button button)
				continue;

            button.AddThemeFontSizeOverride(FONT_SIZE, 1);
            button.AddThemeFontOverride(FONT, BlankFont);
            button.IconAlignment = HorizontalAlignment.Center;
        }

		EditorMainScreenButtons.AddThemeConstantOverride(SEPARATION, 0);
		EditorMainScreenButtons.ChildEnteredTree += OnEditorSceneButtonChildEnteredTree;
	}

	void RevertMainScreenButtonsChanges()
	{
        EditorMainScreenButtons.ChildEnteredTree -= OnEditorSceneButtonChildEnteredTree;
		EditorMainScreenButtons.RemoveThemeConstantOverride(SEPARATION);

        // Move the main screen buttons back to the editor titlebar and restore their text

        EditorSceneTabsHBox.RemoveChild(EditorMainScreenButtons);
        EditorTitleBar.AddChild(EditorMainScreenButtons);
        EditorTitleBar.MoveChild(EditorMainScreenButtons, 2);

        foreach (var c in EditorMainScreenButtons.GetChildren())
        {
            if (c is not Button button)
                continue;

            button.RemoveThemeFontSizeOverride(FONT_SIZE);
            button.RemoveThemeFontOverride(FONT);
            button.IconAlignment = HorizontalAlignment.Left;
        }
    }

	void ApplyEditorRunBarChanges()
	{
		// Move the editor run bar to the modern titlebar and make it centred

		EditorTitleBar.RemoveChild(EditorRunBar);
		RunBarRoot.AddChild(EditorRunBar);
		EditorRunBar.SizeFlagsHorizontal = Control.SizeFlags.ShrinkCenter;

		ApplyEditorRunBarPanelStyling();
	}

	void RevertEditorRunBarChanges()
	{
		// Move the editor run bar back to the editor titlebar and restore its size flags

		RunBarRoot.RemoveChild(EditorRunBar);
		EditorTitleBar.AddChild(EditorRunBar);
		EditorTitleBar.MoveChild(EditorRunBar, 4);
		EditorRunBar.SizeFlagsHorizontal = RunBarSizeFlagsH;

        RevertEditorRunBarPanelStyling();
    }

	void ApplyEditorRunBarPanelStyling()
	{
		// The "Modern" theme style has poor content margins for the StyleBoxEmpty containing the editor run bar

        var stylebox = EditorRunBarPanel.GetThemeStylebox(PANEL);

        if (stylebox is StyleBoxEmpty empty) // "Modern" theme
        {
            var duplicate = empty.Duplicate() as StyleBoxEmpty;

            duplicate.ContentMarginBottom = 0;
            duplicate.ContentMarginLeft = 0;
            duplicate.ContentMarginTop = 0;
            duplicate.ContentMarginRight = 0;

            EditorRunBarPanel.AddThemeStyleboxOverride(PANEL, duplicate);
        }
    }

	void RevertEditorRunBarPanelStyling()
	{
		// Restore "Modern" theme style content margins

        var stylebox = EditorRunBarPanel.GetThemeStylebox(PANEL);

        if (stylebox is StyleBoxEmpty)
        {
            EditorRunBarPanel.RemoveThemeStyleboxOverride(PANEL);
        }
    }

	void ApplyEditorMenuBarChanges()
	{
		// Move the editor menubar to the modern titlebar and override its size flags and font size

        EditorTitleBar.RemoveChild(EditorMenuBar);
        MenuBarRoot.AddChild(EditorMenuBar);
		EditorMenuBar.SizeFlagsHorizontal = Control.SizeFlags.ShrinkBegin;
		EditorMenuBar.SizeFlagsVertical = Control.SizeFlags.Fill;

        var fontSize = ScaleInt(12);
        EditorMenuBar.AddThemeFontSizeOverride(FONT_SIZE, fontSize);

        //ApplyEditorPopupMenuStyleChanges();
    }

	void RevertEditorMenuBarChanges()
	{
		// Move the editor menubar back to the editor titlebar and restore its size flags and font size

        MenuBarRoot.RemoveChild(EditorMenuBar);
        EditorTitleBar.AddChild(EditorMenuBar);
		EditorTitleBar.MoveChild(EditorMenuBar, 0);
		EditorMenuBar.SizeFlagsHorizontal = MenuBarSizeFlagsH;
		EditorMenuBar.SizeFlagsVertical = MenuBarSizeFlagsV;

        EditorMenuBar.RemoveThemeFontSizeOverride(FONT_SIZE);

        //RevertEditorPopupMenuStyleChanges();
    }

	void ApplyEditorPopupMenuStyleChanges()
	{
        EditorMenuBar.AddThemeStyleboxOverride(NORMAL, MenuBarNormalStyle);
        EditorMenuBar.AddThemeStyleboxOverride(HOVER, MenuBarHoverStyle);
		EditorMenuBar.AddThemeStyleboxOverride(PRESSED, MenuBarSelectedStyle);
		EditorMenuBar.AddThemeStyleboxOverride(HOVER_PRESSED, MenuBarSelectedStyle);

        var fontSize = ScaleInt(12);
        var children = EditorWindow.FindChildren("*", "PopupMenu", true, false);

		foreach (var child in children)
		{
			if (child is not PopupMenu menu)
				continue;

			menu.AddThemeConstantOverride(V_SEPARATION, fontSize);
			menu.AddThemeFontSizeOverride(FONT_SIZE, fontSize);
			menu.AddThemeStyleboxOverride(PANEL, PopupPanelStyle);
			menu.AddThemeStyleboxOverride(SEPARATOR, PopupSeparatorStyle);
		}
    }

	void RevertEditorPopupMenuStyleChanges()
	{
        EditorMenuBar.RemoveThemeStyleboxOverride(NORMAL);
        EditorMenuBar.RemoveThemeStyleboxOverride(HOVER);
        EditorMenuBar.RemoveThemeStyleboxOverride(PRESSED);
        EditorMenuBar.RemoveThemeStyleboxOverride(HOVER_PRESSED);

        var children = EditorWindow.FindChildren("*", "PopupMenu", true, false);

        foreach (var child in children)
        {
            if (child is not PopupMenu menu)
                continue;

            menu.RemoveThemeConstantOverride(V_SEPARATION);
            menu.RemoveThemeFontSizeOverride(FONT_SIZE);
            menu.RemoveThemeStyleboxOverride(PANEL);
            menu.RemoveThemeStyleboxOverride(SEPARATOR);
        }
    }

	void SetTitlebarMargins(int l, int r, int t, int b)
	{
		ModernTitlebar.AddThemeConstantOverride(MARGIN_LEFT, l);
		ModernTitlebar.AddThemeConstantOverride(MARGIN_RIGHT, r);
		ModernTitlebar.AddThemeConstantOverride(MARGIN_TOP, t);
		ModernTitlebar.AddThemeConstantOverride(MARGIN_BOTTOM, b);
    }

	void SetWindowButtonMargins(int l, int r, int t, int b)
	{
        WindowButtons.AddThemeConstantOverride(MARGIN_LEFT, l);
        WindowButtons.AddThemeConstantOverride(MARGIN_RIGHT, r);
        WindowButtons.AddThemeConstantOverride(MARGIN_TOP, t);
        WindowButtons.AddThemeConstantOverride(MARGIN_BOTTOM, b);
    }

    Color GetBackgroundColor()
    {
        var stylebox = EditorBaseControl.GetThemeStylebox(PANEL);

        if (stylebox is StyleBoxFlat flat)
        {
            var c = flat.BgColor;
            c *= EditorBaseControl.SelfModulate;
            c *= EditorBaseControl.Modulate;
            return c;
        }

        return default;
    }

    Color GetTextColor()
    {
        return EditorBaseControl.GetThemeColor(FONT_COLOR, EDITOR);
    }

	Color GetPopupPanelColor()
	{
		var textColor = GetTextColor();
		return textColor.Luminance > 0.5f ? BackgroundColor.Lightened(0.035f) : BackgroundColor.Darkened(0.15f);
	}

	float ScaleFloat(float value)
	{
		return value * ScreenScale;
	}

	int ScaleInt(int value)
	{
		return Mathf.RoundToInt((float)value * ScreenScale);
	}

    public void OnBeforeSerialize()
    {
        MinimiseButton.Pressed -= OnMinimisePressed;
        MaximiseButton.Pressed -= OnMaximisePressed;
        CloseButton.Pressed -= OnClosePressed;
		EditorWindow.SizeChanged -= OnWindowSizeChanged;
		DragButton.ButtonDown -= OnDragPressed;
		DragButton.GuiInput -= OnDragGuiInput;
        
        EditorMainScreenButtons.ChildEnteredTree -= OnEditorSceneButtonChildEnteredTree;

        GetTree().NodeAdded -= OnSceneTreeNodeAdded;

        SetMeta(META_ORIGINAL_PROC, WindowFrameRemover.OriginalProc);
        SetMeta(META_ORIGINAL_STYLE, WindowFrameRemover.OriginalStyle);
		
        WindowFrameRemover.RevertWindowProcOnly();
    }

    public void OnAfterDeserialize()
    {
        MinimiseButton.Pressed += OnMinimisePressed;
        MaximiseButton.Pressed += OnMaximisePressed;
        CloseButton.Pressed += OnClosePressed;
		EditorWindow.SizeChanged += OnWindowSizeChanged;
		DragButton.ButtonDown += OnDragPressed;
		DragButton.GuiInput += OnDragGuiInput;

        EditorMainScreenButtons.ChildEnteredTree += OnEditorSceneButtonChildEnteredTree;

        GetTree().NodeAdded += OnSceneTreeNodeAdded;

        var proc = (nint)GetMeta(META_ORIGINAL_PROC, 0).As<long>();
		var style = GetMeta(META_ORIGINAL_STYLE, 0).As<long>();
		WindowFrameRemover.Apply(proc, style);
    }
}
#endif
