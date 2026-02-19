#include "moderntitlebar.hpp"

using namespace godot;

// WINDOW ICONS
static constexpr const char* CloseIcon = u8"\uE8BB";
static constexpr const char* MinimiseIcon = u8"\uE921";
static constexpr const char* MaximizeIcon = u8"\uE922";
static constexpr const char* RestoreIcon = u8"\uE923";

void mtb::ModernTitleBar::_enter_tree()
{
	auto* ds = DisplayServer::get_singleton();
	auto* loader = ResourceLoader::get_singleton();

	// Windows DPI is 96 at 100% scaling
	_screen_scale = ds->screen_get_dpi() / 96.0f;
	_blank_font = loader->load("res://addons/moderntitlebar/AdobeBlank.ttf");

	// Setup titlebar
	Ref<PackedScene> titleBarPrefab = loader->load("res://addons/moderntitlebar/moderntitlebar.scn");
	_modern_titlebar = Object::cast_to<MarginContainer>(titleBarPrefab.ptr()->instantiate());
	_menu_bar_root = _modern_titlebar->get_node<Control>("%MenuBarRoot");
	_run_bar_root = _modern_titlebar->get_node<Control>("%RunBarRoot");
	_drag_button = _modern_titlebar->get_node<Button>("%Drag");
	_icon_padding = _modern_titlebar->get_node<MarginContainer>("%IconPadding");
	_drag_button->connect("button_down", Callable(this, "_on_drag_pressed"));
	_drag_button->connect("gui_input", Callable(this, "_on_drag_gui_input"));

	// Setup window buttons
	Ref<PackedScene> buttonPrefab = loader->load("res://addons/moderntitlebar/window_buttons.scn");
	_window_buttons = Object::cast_to<MarginContainer>(buttonPrefab.ptr()->instantiate());
	_window_buttons_hbox = _window_buttons->get_node<HBoxContainer>("%HBoxContainer");
	_minimise_button = _window_buttons->get_node<Button>("%Minimise");
	_maximise_button = _window_buttons->get_node<Button>("%Maximise");
	_close_button = _window_buttons->get_node<Button>("%Close");

	// 45:29 aspect
	_window_button_size = Vector2(scale_int(45), scale_int(29));

	_minimise_button->set_custom_minimum_size(_window_button_size);
	_maximise_button->set_custom_minimum_size(_window_button_size);
	_close_button->set_custom_minimum_size(_window_button_size);

	_minimise_button->connect("pressed", Callable(this, "_on_minimise_pressed"));
	_maximise_button->connect("pressed", Callable(this, "_on_maximise_pressed"));
	_close_button->connect("pressed", Callable(this, "_on_close_pressed"));

	int buttonFontSize = scale_int(10);

	_minimise_button->add_theme_font_size_override("font_size", buttonFontSize);
	_maximise_button->add_theme_font_size_override("font_size", buttonFontSize);
	_close_button->add_theme_font_size_override("font_size", buttonFontSize);
	_window_buttons_hbox->add_theme_constant_override("separation", scale_int(1));

	// Get handles to editor control nodes
	_editor_window = get_tree()->get_root();
	_editor_node = _editor_window->get_child(0);
	_editor_base_control = Object::cast_to<Panel>(EditorInterface::get_singleton()->get_base_control());
	_editor_main_vbox = Object::cast_to<VBoxContainer>(_editor_base_control->get_child(0));
	_editor_title_bar = Object::cast_to<Control>(_editor_base_control->find_child("*EditorTitleBar*", true, false));
	_editor_menu_bar = Object::cast_to<MenuBar>(_editor_title_bar->find_child("*MenuBar*", true, false));
	_editor_run_bar = Object::cast_to<Control>(_editor_title_bar->find_child("*EditorRunBar*", true, false));
	_editor_run_bar_panel = Object::cast_to<PanelContainer>(_editor_run_bar->find_child("*PanelContainer*", true, false));
	_editor_main_screen_buttons = Object::cast_to<HBoxContainer>(_editor_title_bar->find_child("*EditorMainScreenButtons*", true, false));
	_editor_scene_tabs = Object::cast_to<Control>(_editor_base_control->find_child("*EditorSceneTabs*", true, false));
	_editor_scene_tabs_hbox = Object::cast_to<HBoxContainer>(_editor_scene_tabs->find_child("*HBoxContainer*", true, false));

	// Listen to window size changed
	_editor_window->connect("size_changed", Callable(this, "_on_window_size_changed"));

	// Add plugin controls to editor
	_editor_main_vbox->add_child(_modern_titlebar);
	_editor_main_vbox->move_child(_modern_titlebar, 0);
	_editor_base_control->add_child(_window_buttons);
	_editor_base_control->move_child(_window_buttons, 1);


	// Create plugin styling
	_background_color = get_background_color();
	create_plugin_styleboxes();

	// Apply changes to editor controls
	apply_main_screen_buttons_changes();
	apply_editor_run_bar_changes();
	apply_editor_menu_bar_changes();
	apply_editor_popup_menu_style_changes();

	// Hide default title bar
	_editor_title_bar->hide();

	// Theme native titlebars of editor subwindows
	apply_titlebar_colors(true);

	// Remove native windows titlebar
	win::apply_window_frame_remover();

	// Update
	_on_window_size_changed();

	auto pos = ds->window_get_position(0);
	pos.y = MAX(pos.y, 0);
	ds->window_set_position(pos);

	get_tree()->connect("node_added", Callable(this, "_on_scene_tree_node_added"));
}

void mtb::ModernTitleBar::_exit_tree()
{
	revert_editor_popup_menu_style_changes();
	revert_editor_menu_bar_changes();
	revert_main_screen_buttons_changes();
	revert_editor_run_bar_changes();

	_editor_main_vbox->remove_child(_modern_titlebar);
	_modern_titlebar->queue_free();
	_editor_base_control->remove_child(_window_buttons);
	_window_buttons->queue_free();

	_editor_title_bar->show();
	win::revert_titlebar_colors();
	win::revert_window_frame_remover();
}

void mtb::ModernTitleBar::_process(double delta)
{
	apply_titlebar_colors(true);

	auto stylebox = _editor_run_bar_panel->get_theme_stylebox("panel");
	auto empty = Object::cast_to<StyleBoxEmpty>(stylebox.ptr());
	if (empty && empty->get_content_margin(SIDE_TOP) != 0)
	{
		apply_editor_run_bar_panel_styling();
	}

	auto backgroundColor = get_background_color();
	if (backgroundColor != _background_color)
	{
		_background_color = backgroundColor;
		create_plugin_styleboxes();
		apply_editor_popup_menu_style_changes();
	}
}

void mtb::ModernTitleBar::apply_titlebar_colors(bool skip_main_window)
{
	auto titleColor = get_background_color();
	auto textColor = get_text_color();
	win::apply_titlebar_colors(titleColor, textColor, skip_main_window);
}

void mtb::ModernTitleBar::_on_window_size_changed()
{
	auto max = _editor_window->get_mode() == Window::MODE_MAXIMIZED;
	auto pos = DisplayServer::get_singleton()->window_get_position(0);

	auto mT = max ? Math::abs(pos.x) : scale_int(-3);
	auto mW = max ? Math::abs(pos.x) : scale_int(1);

	auto tbLeft = scale_int(-3);
	auto tbBottom = scale_int(-4);

	set_titlebar_margins(tbLeft, 0, mT, max ? 0 : tbBottom);
	set_window_button_margins(0, mW, mW, 0);

	auto iconPad = max ? scale_int(Math::abs(pos.x) + 4) : scale_int(8);
	_icon_padding->add_theme_constant_override("margin_left", iconPad);

	_maximise_button->set_text(max ? String::utf8(RestoreIcon) : String::utf8(MaximizeIcon));
}

void mtb::ModernTitleBar::_on_minimise_pressed()
{
	_editor_window->set_mode(Window::MODE_MINIMIZED);
}

void mtb::ModernTitleBar::_on_maximise_pressed()
{
	if (_editor_window->get_mode() == Window::MODE_MAXIMIZED)
	{
		_editor_window->set_mode(Window::MODE_WINDOWED);
	}
	else
	{
		_editor_window->set_mode(Window::MODE_MAXIMIZED);
	}
}

void mtb::ModernTitleBar::_on_close_pressed()
{
	mtb::win::request_close_main_editor_window();
}

void mtb::ModernTitleBar::_on_drag_pressed()
{
	_editor_window->start_drag();
}

void mtb::ModernTitleBar::_on_editor_scene_button_child_entered_tree(Node* child)
{
	// Support editor plugins that implement main screens that are enabled after this plugin

	auto button = Object::cast_to<Button>(child);
	if (!button)
	{
		return;
	}

	button->add_theme_font_size_override("font_size", 1);
	button->add_theme_font_override("font", _blank_font);
	button->set_icon_alignment(HORIZONTAL_ALIGNMENT_CENTER);
}

void mtb::ModernTitleBar::_on_drag_gui_input(InputEvent* event)
{
	auto mouse = Object::cast_to<InputEventMouseButton>(event);
	if (!mouse)
	{
		return;
	}

	if (mouse->get_button_index() != MOUSE_BUTTON_RIGHT)
	{
		return;
	}

	if (!mouse->is_pressed())
	{
		return;
	}

	// show window context menu
	win::show_system_menu_for_main_editor_window();
}

void mtb::ModernTitleBar::_on_scene_tree_node_added(Node* node)
{
	auto popup = Object::cast_to<PopupMenu>(node);

	if (!popup)
	{
		return;
	}

	if (_editor_menu_bar->is_ancestor_of(popup))
	{
		return;
	}

	auto fontSize = scale_int(12);
	
	popup->add_theme_constant_override("v_separation", fontSize);
	popup->add_theme_font_size_override("font_size", fontSize);
	popup->add_theme_stylebox_override("panel", _popup_panel_style);
	popup->add_theme_stylebox_override("separator", _popup_separator_style);
}

void mtb::ModernTitleBar::create_plugin_styleboxes()
{
	_menu_bar_hover_style = _editor_menu_bar->get_theme_stylebox("normal", "Editor").ptr()->duplicate();

	auto radius = scale_int(4);

	_menu_bar_hover_style.ptr()->set_corner_radius_all(radius);
	_menu_bar_hover_style.ptr()->set_corner_detail(8);
	_menu_bar_hover_style.ptr()->set_border_width_all(0);
	_menu_bar_hover_style.ptr()->set_draw_center(true);
	_menu_bar_hover_style.ptr()->set_bg_color(get_popup_panel_color());

	_menu_bar_normal_style = _menu_bar_hover_style.ptr()->duplicate();
	_menu_bar_normal_style.ptr()->set_draw_center(false);

	_menu_bar_selected_style = _menu_bar_hover_style.ptr()->duplicate();
	_menu_bar_selected_style.ptr()->set_corner_radius(CORNER_BOTTOM_LEFT, 0);
	_menu_bar_selected_style.ptr()->set_corner_radius(CORNER_BOTTOM_RIGHT, 0);

	auto contentMargin = scale_int(4);
	_popup_panel_style = Ref(memnew(StyleBoxFlat));
	_popup_panel_style.ptr()->set_content_margin_all(contentMargin);
	_popup_panel_style.ptr()->set_bg_color(_menu_bar_hover_style.ptr()->get_bg_color());

	_popup_separator_style = Ref(memnew(StyleBoxFlat));
	_popup_separator_style.ptr()->set_bg_color(Color(0,0,0,0));
	auto separatorColor = get_text_color();
	separatorColor.a = 0.098f;
	_popup_separator_style.ptr()->set_border_color(separatorColor);
	_popup_separator_style.ptr()->set_border_width(SIDE_TOP, scale_int(1));
}

void mtb::ModernTitleBar::apply_main_screen_buttons_changes()
{
	// Move the main screen buttons to the editor scene tabs control and hide their text
	_editor_title_bar->remove_child(_editor_main_screen_buttons);
	_editor_scene_tabs_hbox->add_child(_editor_main_screen_buttons);
	_editor_scene_tabs_hbox->move_child(_editor_main_screen_buttons, 3);

	auto children = _editor_main_screen_buttons->get_children();

	for (int i = 0; i < children.size(); i++)
	{
		Button* button = Object::cast_to<Button>(children[i]);
		if (!button)
		{
			continue;
		}
		button->add_theme_font_size_override("font_size", 1);
		button->add_theme_font_override("font", _blank_font);
		button->set_icon_alignment(HORIZONTAL_ALIGNMENT_CENTER);
	}

	_editor_main_screen_buttons->add_theme_constant_override("separation", 0);
	_editor_main_screen_buttons->connect("child_entered_tree", Callable(this, "_on_editor_scene_button_child_entered_tree"));
}

void mtb::ModernTitleBar::revert_main_screen_buttons_changes()
{
	// Disconnect child entered tree?

	_editor_main_screen_buttons->remove_theme_constant_override("separation");

	// Move the main screen buttons back to the editor titlebar and restore their text

	_editor_scene_tabs_hbox->remove_child(_editor_main_screen_buttons);
	_editor_title_bar->add_child(_editor_main_screen_buttons);
	_editor_title_bar->move_child(_editor_main_screen_buttons, 2);

	auto children = _editor_main_screen_buttons->get_children();

	for (int i = 0; i < children.size(); i++)
	{
		auto button = Object::cast_to<Button>(children[i]);
		if (!button)
		{
			continue;
		}

		button->remove_theme_font_size_override("font_size");
		button->remove_theme_font_override("font");
		button->set_icon_alignment(HORIZONTAL_ALIGNMENT_LEFT);
	}
}

void mtb::ModernTitleBar::apply_editor_run_bar_changes()
{
	_editor_title_bar->remove_child(_editor_run_bar);
	_run_bar_root->add_child(_editor_run_bar);
	_editor_run_bar->set_meta("h_size_flags", _editor_run_bar->get_h_size_flags());
	_editor_run_bar->set_h_size_flags(Control::SIZE_SHRINK_CENTER);

	apply_editor_run_bar_panel_styling();
}

void mtb::ModernTitleBar::revert_editor_run_bar_changes()
{
	// Move the editor run bar back to the editor titlebar and restore its size flags
	
	_run_bar_root->remove_child(_editor_run_bar);
	_editor_title_bar->add_child(_editor_run_bar);
	_editor_title_bar->move_child(_editor_run_bar, 4);
	BitField<Control::SizeFlags> h = _editor_menu_bar->get_meta("h_size_flags");
	_editor_title_bar->set_h_size_flags(h);

	revert_editor_run_bar_panel_styling();
}

void mtb::ModernTitleBar::apply_editor_run_bar_panel_styling()
{
	// The "Modern" theme style has poor content margins for the StyleBoxEmpty containing the editor run bar

	auto stylebox = _editor_run_bar_panel->get_theme_stylebox("panel");

	auto empty = Object::cast_to<StyleBoxEmpty>(stylebox.ptr());

	if (!empty)
	{
		return;
	}

	Ref<StyleBoxEmpty> duplicate = empty->duplicate();

	duplicate.ptr()->set_content_margin_all(0);
	_editor_run_bar_panel->add_theme_stylebox_override("panel", duplicate);
}

void mtb::ModernTitleBar::revert_editor_run_bar_panel_styling()
{
	auto stylebox = _editor_run_bar_panel->get_theme_stylebox("panel");
	auto empty = Object::cast_to<StyleBoxEmpty>(stylebox.ptr());

	if (empty)
	{
		_editor_run_bar_panel->remove_theme_stylebox_override("panel");
	}
}

void mtb::ModernTitleBar::apply_editor_menu_bar_changes()
{
	// Move the editor menubar to the modern titlebar and override its size flags and font size
	_editor_title_bar->remove_child(_editor_menu_bar);
	_menu_bar_root->add_child(_editor_menu_bar);

	_editor_menu_bar->set_meta("h_size_flags", _editor_menu_bar->get_h_size_flags());
	_editor_menu_bar->set_meta("v_size_flags", _editor_menu_bar->get_v_size_flags());

	_editor_menu_bar->set_h_size_flags(Control::SIZE_SHRINK_BEGIN);
	_editor_menu_bar->set_v_size_flags(Control::SIZE_FILL);

	auto fontSize = scale_int(12);
	_editor_menu_bar->add_theme_font_size_override("font_size", fontSize);
}

void mtb::ModernTitleBar::revert_editor_menu_bar_changes()
{
	// Move the editor menubar back to the editor titlebar and restore its size flags and font size
	_menu_bar_root->remove_child(_editor_menu_bar);
	_editor_title_bar->add_child(_editor_menu_bar);
	_editor_title_bar->move_child(_editor_menu_bar, 0);

	// Size flags

	BitField<Control::SizeFlags> h = _editor_menu_bar->get_meta("h_size_flags");
	BitField<Control::SizeFlags> v = _editor_menu_bar->get_meta("v_size_flags");
	_editor_menu_bar->set_h_size_flags(h);
	_editor_menu_bar->set_v_size_flags(v);

	_editor_menu_bar->remove_theme_font_size_override("font_size");
}

void mtb::ModernTitleBar::apply_editor_popup_menu_style_changes()
{
	_editor_menu_bar->add_theme_stylebox_override("normal", _menu_bar_normal_style);
	_editor_menu_bar->add_theme_stylebox_override("hover", _menu_bar_hover_style);
	_editor_menu_bar->add_theme_stylebox_override("pressed", _menu_bar_selected_style);
	_editor_menu_bar->add_theme_stylebox_override("hover_pressed", _menu_bar_selected_style);

	auto fontSize = scale_int(12);
	auto children = _editor_window->find_children("*", "PopupMenu", true, false);

	for (int i = 0; i < children.size(); i++)
	{
		PopupMenu* menu = Object::cast_to<PopupMenu>(children[i]);
		if (!menu)
		{
			continue;
		}

		menu->add_theme_constant_override("v_separation", fontSize);
		menu->add_theme_font_size_override("font_size", fontSize);
		menu->add_theme_stylebox_override("panel", _popup_panel_style);
		menu->add_theme_stylebox_override("separator", _popup_separator_style);
	}
}

void mtb::ModernTitleBar::revert_editor_popup_menu_style_changes()
{
	_editor_menu_bar->remove_theme_stylebox_override("normal");
	_editor_menu_bar->remove_theme_stylebox_override("hover");
	_editor_menu_bar->remove_theme_stylebox_override("pressed");
	_editor_menu_bar->remove_theme_stylebox_override("hover_pressed");

	auto children = _editor_window->find_children("*", "PopupMenu", true, false);

	for (int i = 0; i < children.size(); i++)
	{
		PopupMenu* menu = Object::cast_to<PopupMenu>(children[i]);
		if (!menu)
		{
			continue;
		}

		menu->remove_theme_constant_override("v_separation");
		menu->remove_theme_font_size_override("font_size");
		menu->remove_theme_stylebox_override("panel");
		menu->remove_theme_stylebox_override("separator");
	}
}

void mtb::ModernTitleBar::set_titlebar_margins(const int l, const int r, const int t, const int b)
{
	_modern_titlebar->add_theme_constant_override("margin_left", l);
	_modern_titlebar->add_theme_constant_override("margin_right", r);
	_modern_titlebar->add_theme_constant_override("margin_top", t);
	_modern_titlebar->add_theme_constant_override("margin_bottom", b);
}

void mtb::ModernTitleBar::set_window_button_margins(const int l, const int r, const int t, const int b)
{
	_window_buttons->add_theme_constant_override("margin_left", l);
	_window_buttons->add_theme_constant_override("margin_right", r);
	_window_buttons->add_theme_constant_override("margin_top", t);
	_window_buttons->add_theme_constant_override("margin_bottom", b);
}

Color mtb::ModernTitleBar::get_background_color()
{
	Ref<StyleBox> stylebox = _editor_base_control->get_theme_stylebox("panel");
	StyleBoxFlat* flat = Object::cast_to<StyleBoxFlat>(stylebox.ptr());

	if (!flat)
	{
		return Color();
	}

	auto c = flat->get_bg_color();
	c *= _editor_base_control->get_self_modulate();
	c *= _editor_base_control->get_modulate();
	return c;
}

Color mtb::ModernTitleBar::get_text_color()
{
	return _editor_base_control->get_theme_color("font_color", "Editor");
}

Color mtb::ModernTitleBar::get_popup_panel_color()
{
	auto textColor = get_text_color();
	return textColor.get_luminance() > 0.5f ? _background_color.lightened(0.035f) : _background_color.darkened(0.15f);
}

float mtb::ModernTitleBar::scale_float(const float value) const
{
	return value * _screen_scale;
}

int mtb::ModernTitleBar::scale_int(const int value) const
{
	return (int)Math::round((float)value * _screen_scale);
}

void mtb::ModernTitleBar::_bind_methods()
{
	ClassDB::bind_method(D_METHOD("_on_drag_pressed"), &mtb::ModernTitleBar::_on_drag_pressed);
	ClassDB::bind_method(D_METHOD("_on_minimise_pressed"), &mtb::ModernTitleBar::_on_minimise_pressed);
	ClassDB::bind_method(D_METHOD("_on_maximise_pressed"), &mtb::ModernTitleBar::_on_maximise_pressed);
	ClassDB::bind_method(D_METHOD("_on_close_pressed"), &mtb::ModernTitleBar::_on_close_pressed);

	ClassDB::bind_method(
		D_METHOD("_on_drag_gui_input", "event"),
		&mtb::ModernTitleBar::_on_drag_gui_input
	);

	ClassDB::bind_method(
		D_METHOD("_on_window_size_changed"),
		&mtb::ModernTitleBar::_on_window_size_changed
	);

	ClassDB::bind_method(
		D_METHOD("_on_scene_tree_node_added", "node"),
		&mtb::ModernTitleBar::_on_scene_tree_node_added
	);

	ClassDB::bind_method(
		D_METHOD("_on_editor_scene_button_child_entered_tree", "child"),
		&mtb::ModernTitleBar::_on_editor_scene_button_child_entered_tree
	);
}
