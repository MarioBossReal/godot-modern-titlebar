#pragma once

#include <godot_cpp/classes/editor_plugin.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/input_event.hpp>
#include <godot_cpp/classes/input_event_mouse_button.hpp>
#include <godot_cpp/classes/margin_container.hpp>
#include <godot_cpp/classes/button.hpp>
#include <godot_cpp/classes/h_box_container.hpp>
#include <godot_cpp/classes/v_box_container.hpp>
#include <godot_cpp/classes/style_box_flat.hpp>
#include <godot_cpp/classes/style_box_empty.hpp>
#include <godot_cpp/classes/font_file.hpp>
#include <godot_cpp/classes/window.hpp>
#include <godot_cpp/classes/panel.hpp>
#include <godot_cpp/classes/panel_container.hpp>
#include <godot_cpp/classes/menu_bar.hpp>
#include <godot_cpp/classes/display_server.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/packed_scene.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/editor_interface.hpp>
#include <godot_cpp/classes/popup_menu.hpp>
#include <godot_cpp/core/memory.hpp>
#include "window_closer.hpp"
#include "window_styler.hpp"
#include "window_context_menu.hpp"
#include "window_frame_remover.hpp"

using namespace godot;

namespace mtb 
{
	class ModernTitleBar : public godot::EditorPlugin
	{
		GDCLASS(ModernTitleBar, EditorPlugin)

		protected:
		static void _bind_methods();
		
		public:
		void _enter_tree() override;
		void _exit_tree() override;
		void _process(double delta) override;
		void _notification(int what);

		private:
		void apply_titlebar_colors(bool skip_main_window);
		void _on_window_size_changed();
		void _on_minimise_pressed();
		void _on_maximise_pressed();
		void _on_close_pressed();
		void _on_drag_pressed();
		void _on_editor_scene_button_child_entered_tree(Node* child);
		void _on_drag_gui_input(InputEvent* event);
		void _on_scene_tree_node_added(Node* node);
		void create_plugin_styleboxes();
		void apply_main_screen_buttons_changes();
		void revert_main_screen_buttons_changes();
		void apply_editor_run_bar_changes();
		void revert_editor_run_bar_changes();
		void apply_editor_run_bar_panel_styling();
		void revert_editor_run_bar_panel_styling();
		void apply_editor_menu_bar_changes();
		void revert_editor_menu_bar_changes();
		void apply_editor_popup_menu_style_changes();
		void revert_editor_popup_menu_style_changes();
		void set_titlebar_margins(const int l, const int r, const int t, const int b);
		void set_window_button_margins(const int l, const int r, const int t, const int b);
		Color get_background_color();
		Color get_text_color();
		Color get_popup_panel_color();
		float scale_float(const float value) const;
		int scale_int(const int value) const;
		void calculate_screen_scale();

		// SIZE / SCALE
		Vector2 _window_button_size;
		float _screen_scale;
		Color _background_color;

		// PLUGIN CONTROLS
		MarginContainer* _modern_titlebar;
		Control* _menu_bar_root;
		Control* _run_bar_root;
		Button* _drag_button;
		MarginContainer* _window_buttons;
		HBoxContainer* _window_buttons_hbox;
		Button* _minimise_button;
		Button* _maximise_button;
		Button* _close_button;
		MarginContainer* _icon_padding;

		// PLUGINSTYLING

		Ref<StyleBoxFlat> _menu_bar_normal_style;
		Ref<StyleBoxFlat> _menu_bar_hover_style;
		Ref<StyleBoxFlat> _menu_bar_selected_style;
		Ref<StyleBoxFlat> _popup_panel_style;
		Ref<StyleBoxFlat> _popup_separator_style;

		// A workaround to the fact that the EditorMainScreen's functionality depends extremely heavily on the buttons' names
		Ref<FontFile> _blank_font;

		// EDITOR CONTROLS
		Window* _editor_window;
		Node* _editor_node;
		VBoxContainer* _editor_main_vbox;
		Panel* _editor_base_control;
		Control* _editor_title_bar;
		MenuBar* _editor_menu_bar;
		Control* _editor_run_bar;
		PanelContainer* _editor_run_bar_panel;
		HBoxContainer* _editor_main_screen_buttons;
		Control* _editor_scene_tabs;
		HBoxContainer* _editor_scene_tabs_hbox;
	};
}