#include "xhui.h"
#include "config.h"
#include "Menu.h"
#include "Panel.h"
#include "Window.h"
#include "../kabaexport/KabaExporter.h"
#include "../base/callable.h"


#define KABA_EXPORT_HUI


namespace hui{
#ifdef KABA_EXPORT_HUI
	xfer<xhui::Menu> create_menu_from_source(const string &source, xhui::Panel*);
#endif
}


#ifdef KABA_EXPORT_HUI

	// capturing all function pointers as pointers or references!!!

	/*void hui_set_idle_function_kaba(Callable<void()> &c) {
		xhui::set_idle_function([&c]{ c(); });
	}*/
	int hui_run_later_kaba(float dt, Callable<void()> &c) {
		return xhui::run_later(dt, [&c]{ c(); });
	}
	int hui_run_repeated_kaba(float dt, Callable<void()> &c) {
		return xhui::run_repeated(dt, [&c]{ c(); });
	}
	class KabaPanelWrapper : public xhui::Panel {
	public:
		void _kaba_event(const string &id, Callable<void()> &c) {
			event(id, [&c]{ c(); });
		}
		void _kaba_event_x(const string &id, const string &msg, void *f) {
			if (msg == "hui:draw"){
				auto &ff = *(Callable<void(Painter*)>*)f;
				event_xp(id, msg, [&ff](Painter *p){ ff(p); });
			}else{
				auto &ff = *(Callable<void()>*)f;
				event_x(id, msg, [&ff]{ ff(); });
			}
		}
		void _kaba_event_xp(const string &id, const string &msg, void *f) {
			auto &ff = *(Callable<void(Painter*)>*)f;
			event_xp(id, msg, [&ff](Painter *p){ ff(p); });
		}
	};
#endif

void _dummy() {}


void export_package_xhui(kaba::Exporter* e) {
#if 0
	e->declare_class_size("Menu", sizeof(xhui::Menu));
	e->link_class_func("Menu.__init__", &xhui::Menu::__init__);
	e->link_class_func("Menu.popup", &xhui::Menu::open_popup);
	e->link_class_func("Menu.add", &xhui::Menu::add_item);
/*	e->link_class_func("Menu.add_with_image", &xhui::Menu::add_with_image);
	e->link_class_func("Menu.add_checkable", &xhui::Menu::add_checkable);
	e->link_class_func("Menu.add_separator", &xhui::Menu::add_separator);
	e->link_class_func("Menu.add_sub_menu", &xhui::Menu::add_sub_menu);*/
	e->link_class_func("Menu.enable", &xhui::Menu::enable);
//	e->link_class_func("Menu.check", &xhui::Menu::check);
#endif


/*	e->declare_class_size("Toolbar", sizeof(xhui::Toolbar));
	e->link_class_func("Toolbar.set_by_id", &xhui::Toolbar::set_by_id);
	e->link_class_func("Toolbar.from_source", &xhui::Toolbar::from_source);*/


	{
		xhui::Control ctrl("");
		e->declare_class_size("Control", sizeof(xhui::Control));
		e->link_virtual("Control.add_child", &xhui::Control::add_child, &ctrl);
		e->link_virtual("Control.remove_child", &xhui::Control::remove_child, &ctrl);
		e->link_virtual("Control.set_string", &xhui::Control::set_string, &ctrl);
		e->link_virtual("Control.add_string", &xhui::Control::add_string, &ctrl);
		e->link_virtual("Control.set_cell", &xhui::Control::set_cell, &ctrl);
		e->link_virtual("Control.reset", &xhui::Control::reset, &ctrl);
		e->link_virtual("Control.set_int", &xhui::Control::set_int, &ctrl);
		e->link_virtual("Control.set_float", &xhui::Control::set_float, &ctrl);

		e->link_virtual("Control.check", &xhui::Control::check, &ctrl);
		e->link_virtual("Control.expand", &xhui::Control::expand, &ctrl);
		e->link_virtual("Control.set_color", &xhui::Control::set_color, &ctrl);
		e->link_virtual("Control.get_string", &xhui::Control::get_string, &ctrl);
		e->link_virtual("Control.get_cell", &xhui::Control::get_cell, &ctrl);
		e->link_virtual("Control.get_int", &xhui::Control::get_int, &ctrl);
		e->link_virtual("Control.get_float", &xhui::Control::get_float, &ctrl);
		e->link_virtual("Control.is_checked", &xhui::Control::is_checked, &ctrl);
		e->link_virtual("Control.get_color", &xhui::Control::get_color, &ctrl);
		e->link_virtual("Control.get_selection", &xhui::Control::get_selection, &ctrl);
		e->link_virtual("Control.enable", &xhui::Control::enable, &ctrl);
		e->link_virtual("Control.set_option", &xhui::Control::set_option, &ctrl);
		e->link_virtual("Control.get_children", &xhui::Control::get_children, &ctrl);
		e->link_virtual("Control.set_float", &xhui::Control::set_float, &ctrl);
		e->link_virtual("Control.set_float", &xhui::Control::set_float, &ctrl);

		e->link_virtual("Control.on_mouse_move", &xhui::Control::on_mouse_move, &ctrl);
		e->link_virtual("Control.on_mouse_wheel", &xhui::Control::on_mouse_wheel, &ctrl);
		e->link_virtual("Control.on_mouse_enter", &xhui::Control::on_mouse_enter, &ctrl);
		e->link_virtual("Control.on_mouse_leave", &xhui::Control::on_mouse_leave, &ctrl);
		e->link_virtual("Control.on_left_button_down", &xhui::Control::on_left_button_down, &ctrl);
		e->link_virtual("Control.on_middle_button_down", &xhui::Control::on_middle_button_down, &ctrl);
		e->link_virtual("Control.on_right_button_down", &xhui::Control::on_right_button_down, &ctrl);
		e->link_virtual("Control.on_left_button_up", &xhui::Control::on_left_button_up, &ctrl);
		e->link_virtual("Control.on_middle_button_up", &xhui::Control::on_middle_button_up, &ctrl);
		e->link_virtual("Control.on_right_button_up", &xhui::Control::on_right_button_up, &ctrl);
		e->link_virtual("Control.on_left_double_click", &xhui::Control::on_left_double_click, &ctrl);
		e->link_virtual("Control.on_key_down", &xhui::Control::on_key_down, &ctrl);
		e->link_virtual("Control.on_key_up", &xhui::Control::on_key_up, &ctrl);
		e->link_virtual("Control.on_key_char", &xhui::Control::on_key_char, &ctrl);

		e->link_virtual("Control.set_option", &xhui::Control::set_option, &ctrl);
		e->link_virtual("Control.get_children", &xhui::Control::get_children, &ctrl);

		e->link_virtual("Control._draw", &xhui::Control::_draw, &ctrl);
		e->link_virtual("Control.get_greed_factor", &xhui::Control::get_greed_factor, &ctrl);
		e->link_virtual("Control.get_content_min_size", &xhui::Control::get_content_min_size, &ctrl);
		e->link_virtual("Control.negotiate_area", &xhui::Control::negotiate_area, &ctrl);
	}


	{
		xhui::Panel panel("");
		e->declare_class_size("Panel", sizeof(xhui::Panel));
		//	e->declare_class_element("Panel.win", &xhui::Panel::win);
		e->link_class_func("Panel.__init__", &kaba::generic_init_ext<xhui::Panel, const string&>);
		e->link_virtual("Panel.__delete__", &kaba::generic_virtual<KabaPanelWrapper>::__delete__, &panel);
		/*	e->link_class_func("Panel.set_border_width", &xhui::Panel::set_border_width);
			e->link_class_func("Panel.set_decimals", &xhui::Panel::set_decimals);
			e->link_class_func("Panel.activate", &xhui::Panel::activate);
			e->link_class_func("Panel.is_active", &xhui::Panel::is_active);*/
		e->link_class_func("Panel.from_source", &xhui::Panel::from_source);
		/*e->link_class_func("Panel.add_button", &xhui::Panel::add_button);
		e->link_class_func("Panel.add_toggle_button", &xhui::Panel::add_toggle_button);
		e->link_class_func("Panel.add_check_box", &xhui::Panel::add_check_box);
		e->link_class_func("Panel.add_label", &xhui::Panel::add_label);
		e->link_class_func("Panel.add_edit", &xhui::Panel::add_edit);
		e->link_class_func("Panel.add_multiline_edit", &xhui::Panel::add_multiline_edit);
		e->link_class_func("Panel.add_group", &xhui::Panel::add_group);
		e->link_class_func("Panel.add_combo_box", &xhui::Panel::add_combo_box);
		e->link_class_func("Panel.add_tab_control", &xhui::Panel::add_tab_control);
		e->link_class_func("Panel.set_target", &xhui::Panel::set_target);
		e->link_class_func("Panel.add_list_view", &xhui::Panel::add_list_view);
		e->link_class_func("Panel.add_tree_view", &xhui::Panel::add_tree_view);
		e->link_class_func("Panel.add_icon_view", &xhui::Panel::add_icon_view);
		e->link_class_func("Panel.add_progress_bar", &xhui::Panel::add_progress_bar);
		e->link_class_func("Panel.add_slider", &xhui::Panel::add_slider);
		e->link_class_func("Panel.add_drawing_area", &xhui::Panel::add_drawing_area);
		e->link_class_func("Panel.add_grid", &xhui::Panel::add_grid);
		e->link_class_func("Panel.add_spin_button", &xhui::Panel::add_spin_button);
		e->link_class_func("Panel.add_radio_button", &xhui::Panel::add_radio_button);
		e->link_class_func("Panel.add_scroller", &xhui::Panel::add_scroller);
		e->link_class_func("Panel.add_expander", &xhui::Panel::add_expander);
		e->link_class_func("Panel.add_separator", &xhui::Panel::add_separator);
		e->link_class_func("Panel.add_paned", &xhui::Panel::add_paned);*/
		e->link_class_func("Panel.embed", &xhui::Panel::embed);
		e->link_class_func("Panel.unembed", &xhui::Panel::unembed);
		e->link_class_func("Panel.set_string", &xhui::Panel::set_string);
		e->link_class_func("Panel.add_string", &xhui::Panel::add_string);
		e->link_class_func("Panel.get_string", &xhui::Panel::get_string);
		e->link_class_func("Panel.set_float", &xhui::Panel::set_float);
		e->link_class_func("Panel.get_float", &xhui::Panel::get_float);
		e->link_class_func("Panel.enable", &xhui::Panel::enable);
		//	e->link_class_func("Panel.is_enabled", &xhui::Panel::is_enabled);
		e->link_class_func("Panel.check", &xhui::Panel::check);
		e->link_class_func("Panel.is_checked", &xhui::Panel::is_checked);
		//	e->link_class_func("Panel.hide_control", &xhui::Panel::hide_control);
		//	e->link_class_func("Panel.delete_control", &xhui::Panel::remove_control);
		e->link_class_func("Panel.set_int", &xhui::Panel::set_int);
		e->link_class_func("Panel.get_int", &xhui::Panel::get_int);
		e->link_class_func("Panel.set_color", &xhui::Panel::set_color);
		e->link_class_func("Panel.get_color", &xhui::Panel::get_color);
		//	e->link_class_func("Panel.set_selection", &xhui::Panel::set_selection);
		//	e->link_class_func("Panel.get_selection", &xhui::Panel::get_selection);
		//	e->link_class_func("Panel.set_image", &xhui::Panel::set_image);
		//	e->link_class_func("Panel.set_cell", &xhui::Panel::set_cell);
		//	e->link_class_func("Panel.get_cell", &xhui::Panel::get_cell);
		e->link_class_func("Panel.set_options", &xhui::Panel::set_options);
		e->link_class_func("Panel.reset", &xhui::Panel::reset);
		//	e->link_class_func("Panel.redraw", &xhui::Panel::redraw);
		//	e->link_class_func("Panel.expand", &xhui::Panel::expand_row);
		e->link_class_func("Panel.expand", &xhui::Panel::expand);
		//	e->link_class_func("Panel.is_expanded", &xhui::Panel::is_expanded);

		e->link_class_func("Panel.event", &KabaPanelWrapper::_kaba_event);
		e->link_class_func("Panel.event_x", &KabaPanelWrapper::_kaba_event_x);
		e->link_class_func("Panel.event_xp", &KabaPanelWrapper::_kaba_event_xp);
		e->link_class_func("Panel.remove_event_handler", &xhui::Panel::remove_event_handler);

		e->link_virtual("Panel._draw", &xhui::Panel::_draw, &panel);
		e->link_virtual("Panel.negotiate_area", &xhui::Panel::negotiate_area, &panel);
		e->link_virtual("Panel.get_content_min_size", &xhui::Panel::get_content_min_size, &panel);
		e->link_virtual("Panel.get_greed_factor", &xhui::Panel::get_greed_factor, &panel);
		e->link_virtual("Panel.get_children", &xhui::Panel::get_children, &panel);
		e->link_virtual("Panel.add_child", &xhui::Panel::add_child, &panel);
		e->link_virtual("Panel.set_option", &xhui::Panel::set_option, &panel);
	}

	{
		xhui::Window win("", 0, 0, xhui::Flags::FAKE);
		e->declare_class_size("Window", sizeof(xhui::Window));
		e->link_class_func("Window.__init__", &kaba::generic_init_ext<xhui::Window, const string&, int, int>);
		e->link_virtual("Window.__delete__", &xhui::Window::__delete__, &win);
		e->link_class_func("Window.destroy", &xhui::Window::request_destroy);
		/*	e->link_class_func("Window.show", &xhui::Window::show);
			e->link_class_func("Window.hide", &xhui::Window::hide);
			e->link_class_func("Window.set_menu", &xhui::Window::set_menu);
			e->link_class_func("Window.toolbar", &xhui::Window::get_toolbar);*/
		e->link_class_func("Window.set_maximized", &xhui::Window::maximize);
		e->link_class_func("Window.is_maximized", &xhui::Window::is_maximized);
		//	e->link_class_func("Window.is_minimized", &xhui::Window::is_minimized);
		//	e->link_class_func("Window.set_id", &xhui::Window::set_id);
		//	e->link_class_func("Window.set_fullscreen", &xhui::Window::set_fullscreen);
		e->link_class_func("Window.set_title", &xhui::Window::set_title);
		e->link_class_func("Window.set_position", &xhui::Window::set_position);
		/*	e->link_class_func("Window.set_size", &xhui::Window::set_size);
			e->link_class_func("Window.get_size", &xhui::Window::get_size);
			e->link_class_func("Window.set_cursor_pos", &xhui::Window::set_cursor_pos);
			e->link_class_func("Window.get_mouse", &xhui::Window::get_mouse);
			e->link_class_func("Window.get_key", &xhui::Window::get_key);*/
		//	e->link_virtual("Window.on_close_request", &xhui::Window::on_close_request, &win);
		e->link_class_func("Window.redraw", &xhui::Window::redraw);
	}


/*	e->link_class_func("GlWindow.__init__", &xhui::NixWindow::__init_ext__);
	e->link_virtual("GlWindow.__delete__", &xhui::NixWindow::__delete__, &win);
	
	e->link_class_func("Dialog.__init__", &xhui::Dialog::__init_ext__);
	e->link_virtual("Dialog.__delete__", &xhui::Dialog::__delete__, &win);*/
	

	xhui::Painter painter(nullptr, nullptr, rect::ID, rect::ID);
	e->link_func("Painter.__init__", &_dummy); // dummy
	e->link_virtual("Painter.__delete__", &kaba::generic_virtual<xhui::Painter>::__delete__, &painter);

	
	// user interface
//	e->link_func("set_idle_function", &hui_set_idle_function_kaba);
	e->link_func("run_later", &hui_run_later_kaba);
	e->link_func("run_repeated", &hui_run_repeated_kaba);
	e->link_func("cancel_runner", &xhui::cancel_runner);
	e->link_func("run", &xhui::run);
#if 0
	e->link_func("fly", &xhui::fly);
	e->link_func("fly_and_wait", &xhui::fly_and_wait);
	/*e->link_func("HuiAddKeyCode", &xhui::AddKeyCode);
	e->link_func("HuiAddCommand", &xhui::AddCommand);*/
	e->link_func("get_event", &xhui::get_event);
	e->link_func("do_single_main_loop", &xhui::Application::do_single_main_loop);
	e->link_func("file_dialog_open", &xhui::file_dialog_open);
	e->link_func("file_dialog_save", &xhui::file_dialog_save);
	e->link_func("file_dialog_dir", &xhui::file_dialog_dir);
	e->link_func("question_box", &xhui::question_box);
	e->link_func("info_box", &xhui::info_box);
	e->link_func("error_box", &xhui::error_box);
	e->link_func("create_menu_from_source", &xhui::create_menu_from_source);
	e->link_func("get_key_name", &xhui::get_key_code_name);
//	e->link_func("get_key_char", &xhui::GetKeyChar);

	e->link_func("open_document", &xhui::open_document);
	e->link_func("make_gui_image", &xhui::set_image);
#endif


	e->link_func("clipboard.paste", &xhui::clipboard::paste);
	e->link_func("clipboard.copy", &xhui::clipboard::copy);

	/*e->declare_class_size("Event", sizeof(xhui::Event));
	e->declare_class_element("Event.id", &xhui::Event::id);
	e->declare_class_element("Event.message", &xhui::Event::message);
	e->declare_class_element("Event.mouse", &xhui::Event::m);
	e->declare_class_element("Event.pressure", &xhui::Event::pressure);
	e->declare_class_element("Event.scroll", &xhui::Event::scroll);
	e->declare_class_element("Event.key", &xhui::Event::key_code);
	e->declare_class_element("Event.width", &xhui::Event::width);
	e->declare_class_element("Event.height", &xhui::Event::height);
	e->declare_class_element("Event.button_l", &xhui::Event::lbut);
	e->declare_class_element("Event.button_m", &xhui::Event::mbut);
	e->declare_class_element("Event.button_r", &xhui::Event::rbut);
	e->declare_class_element("Event.row", &xhui::Event::row);
	e->declare_class_element("Event.column", &xhui::Event::column);*/

#if 0
	// key ids (int)
	add_enum("KEY_CONTROL", TypeInt32, hui::KEY_CONTROL);
	add_enum("KEY_LEFT_CONTROL", TypeInt32, hui::KEY_LCONTROL);
	add_enum("KEY_RIGHT_CONTROL", TypeInt32, hui::KEY_RCONTROL);
	add_enum("KEY_SHIFT", TypeInt32, hui::KEY_SHIFT);
	add_enum("KEY_LEFT_SHIFT", TypeInt32, hui::KEY_LSHIFT);
	add_enum("KEY_RIGHT_SHIFT", TypeInt32, hui::KEY_RSHIFT);
	add_enum("KEY_ALT", TypeInt32, hui::KEY_ALT);
	add_enum("KEY_LEFT_ALT", TypeInt32, hui::KEY_LALT);
	add_enum("KEY_RIGHT_ALT", TypeInt32, hui::KEY_RALT);
	add_enum("KEY_PLUS", TypeInt32, hui::KEY_PLUS);
	add_enum("KEY_MINUS", TypeInt32, hui::KEY_MINUS);
	add_enum("KEY_FENCE", TypeInt32, hui::KEY_FENCE);
	add_enum("KEY_END", TypeInt32, hui::KEY_END);
	add_enum("KEY_PAGE_UP", TypeInt32, hui::KEY_PAGE_UP);
	add_enum("KEY_PAGE_DOWN", TypeInt32, hui::KEY_PAGE_DOWN);
	add_enum("KEY_UP", TypeInt32, hui::KEY_UP);
	add_enum("KEY_DOWN", TypeInt32, hui::KEY_DOWN);
	add_enum("KEY_LEFT", TypeInt32, hui::KEY_LEFT);
	add_enum("KEY_RIGHT", TypeInt32, hui::KEY_RIGHT);
	add_enum("KEY_RETURN", TypeInt32, hui::KEY_RETURN);
	add_enum("KEY_ESCAPE", TypeInt32, hui::KEY_ESCAPE);
	add_enum("KEY_INSERT", TypeInt32, hui::KEY_INSERT);
	add_enum("KEY_DELETE", TypeInt32, hui::KEY_DELETE);
	add_enum("KEY_SPACE", TypeInt32, hui::KEY_SPACE);
	add_enum("KEY_F1", TypeInt32, hui::KEY_F1);
	add_enum("KEY_F2", TypeInt32, hui::KEY_F2);
	add_enum("KEY_F3", TypeInt32, hui::KEY_F3);
	add_enum("KEY_F4", TypeInt32, hui::KEY_F4);
	add_enum("KEY_F5", TypeInt32, hui::KEY_F5);
	add_enum("KEY_F6", TypeInt32, hui::KEY_F6);
	add_enum("KEY_F7", TypeInt32, hui::KEY_F7);
	add_enum("KEY_F8", TypeInt32, hui::KEY_F8);
	add_enum("KEY_F9", TypeInt32, hui::KEY_F9);
	add_enum("KEY_F10", TypeInt32, hui::KEY_F10);
	add_enum("KEY_F11", TypeInt32, hui::KEY_F11);
	add_enum("KEY_F12", TypeInt32, hui::KEY_F12);
	add_enum("KEY_0", TypeInt32, hui::KEY_0);
	add_enum("KEY_1", TypeInt32, hui::KEY_1);
	add_enum("KEY_2", TypeInt32, hui::KEY_2);
	add_enum("KEY_3", TypeInt32, hui::KEY_3);
	add_enum("KEY_4", TypeInt32, hui::KEY_4);
	add_enum("KEY_5", TypeInt32, hui::KEY_5);
	add_enum("KEY_6", TypeInt32, hui::KEY_6);
	add_enum("KEY_7", TypeInt32, hui::KEY_7);
	add_enum("KEY_8", TypeInt32, hui::KEY_8);
	add_enum("KEY_9", TypeInt32, hui::KEY_9);
	add_enum("KEY_A", TypeInt32, hui::KEY_A);
	add_enum("KEY_B", TypeInt32, hui::KEY_B);
	add_enum("KEY_C", TypeInt32, hui::KEY_C);
	add_enum("KEY_D", TypeInt32, hui::KEY_D);
	add_enum("KEY_E", TypeInt32, hui::KEY_E);
	add_enum("KEY_F", TypeInt32, hui::KEY_F);
	add_enum("KEY_G", TypeInt32, hui::KEY_G);
	add_enum("KEY_H", TypeInt32, hui::KEY_H);
	add_enum("KEY_I", TypeInt32, hui::KEY_I);
	add_enum("KEY_J", TypeInt32, hui::KEY_J);
	add_enum("KEY_K", TypeInt32, hui::KEY_K);
	add_enum("KEY_L", TypeInt32, hui::KEY_L);
	add_enum("KEY_M", TypeInt32, hui::KEY_M);
	add_enum("KEY_N", TypeInt32, hui::KEY_N);
	add_enum("KEY_O", TypeInt32, hui::KEY_O);
	add_enum("KEY_P", TypeInt32, hui::KEY_P);
	add_enum("KEY_Q", TypeInt32, hui::KEY_Q);
	add_enum("KEY_R", TypeInt32, hui::KEY_R);
	add_enum("KEY_S", TypeInt32, hui::KEY_S);
	add_enum("KEY_T", TypeInt32, hui::KEY_T);
	add_enum("KEY_U", TypeInt32, hui::KEY_U);
	add_enum("KEY_V", TypeInt32, hui::KEY_V);
	add_enum("KEY_W", TypeInt32, hui::KEY_W);
	add_enum("KEY_X", TypeInt32, hui::KEY_X);
	add_enum("KEY_Y", TypeInt32, hui::KEY_Y);
	add_enum("KEY_Z", TypeInt32, hui::KEY_Z);
	add_enum("KEY_BACKSPACE", TypeInt32, hui::KEY_BACKSPACE);
	add_enum("KEY_TAB", TypeInt32, hui::KEY_TAB);
	add_enum("KEY_HOME", TypeInt32, hui::KEY_HOME);
	add_enum("KEY_NUM_0", TypeInt32, hui::KEY_NUM_0);
	add_enum("KEY_NUM_1", TypeInt32, hui::KEY_NUM_1);
	add_enum("KEY_NUM_2", TypeInt32, hui::KEY_NUM_2);
	add_enum("KEY_NUM_3", TypeInt32, hui::KEY_NUM_3);
	add_enum("KEY_NUM_4", TypeInt32, hui::KEY_NUM_4);
	add_enum("KEY_NUM_5", TypeInt32, hui::KEY_NUM_5);
	add_enum("KEY_NUM_6", TypeInt32, hui::KEY_NUM_6);
	add_enum("KEY_NUM_7", TypeInt32, hui::KEY_NUM_7);
	add_enum("KEY_NUM_8", TypeInt32, hui::KEY_NUM_8);
	add_enum("KEY_NUM_9", TypeInt32, hui::KEY_NUM_9);
	add_enum("KEY_NUM_PLUS", TypeInt32, hui::KEY_NUM_ADD);
	add_enum("KEY_NUM_MINUS", TypeInt32, hui::KEY_NUM_SUBTRACT);
	add_enum("KEY_NUM_MULTIPLY", TypeInt32, hui::KEY_NUM_MULTIPLY);
	add_enum("KEY_NUM_DIVIDE", TypeInt32, hui::KEY_NUM_DIVIDE);
	add_enum("KEY_NUM_COMMA", TypeInt32, hui::KEY_NUM_COMMA);
	add_enum("KEY_NUM_ENTER", TypeInt32, hui::KEY_NUM_ENTER);
	add_enum("KEY_COMMA", TypeInt32, hui::KEY_COMMA);
	add_enum("KEY_DOT", TypeInt32, hui::KEY_DOT);
	add_enum("KEY_LESS", TypeInt32, hui::KEY_LESS);
	add_enum("KEY_SZ", TypeInt32, hui::KEY_SZ);
	add_enum("KEY_AE", TypeInt32, hui::KEY_AE);
	add_enum("KEY_OE", TypeInt32, hui::KEY_OE);
	add_enum("KEY_UE", TypeInt32, hui::KEY_UE);
	add_enum("NUM_KEYS", TypeInt32,hui::NUM_KEYS);
	add_enum("KEY_ANY", TypeInt32, hui::KEY_ANY);
#endif

	e->link("app_config", &xhui::config);
}


