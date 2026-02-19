#include "window_closer.hpp"

using namespace godot;

namespace mtb::win
{
	void request_close_main_editor_window()
	{
		auto ds = DisplayServer::get_singleton();

		HWND hwnd = (HWND)(intptr_t)ds->window_get_native_handle(DisplayServer::HandleType::WINDOW_HANDLE, 0);
	
		::PostMessage(hwnd, WM_CLOSE, 0, 0);
	}
}