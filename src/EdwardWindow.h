/*
 * EdwardWindow.h
 *
 *  Created on: 10 Sept 2023
 *      Author: michi
 */

#ifndef SRC_EDWARDWINDOW_H_
#define SRC_EDWARDWINDOW_H_

#include "lib/hui/hui.h"
#include "lib/os/time.h"
#include "lib/pattern/Observable.h"


class Session;


class EdwardWindow : public obs::Node<hui::Window> {
public:
	EdwardWindow(Session *s);
	~EdwardWindow() override;

	obs::sink in_data_selection_changed;
	obs::sink in_data_changed;
	obs::sink in_action_failed;
	obs::sink in_saved;

	void load_key_codes();

	void idle_function();

	void on_about();
	void on_send_bug_report();

	void on_command(const string &id);
	void on_close();

	void on_draw_gl();
	void on_realize_gl();
	virtual void _cdecl on_key_down() override;
	virtual void _cdecl on_key_up() override;
	virtual void _cdecl on_mouse_move() override;
	virtual void _cdecl on_mouse_wheel() override;
	virtual void _cdecl on_mouse_enter() override;
	virtual void _cdecl on_mouse_leave() override;
	virtual void _cdecl on_left_button_down() override;
	virtual void _cdecl on_left_button_up() override;
	virtual void _cdecl on_middle_button_down() override;
	virtual void _cdecl on_middle_button_up() override;
	virtual void _cdecl on_right_button_down() override;
	virtual void _cdecl on_right_button_up() override;
	void on_event();
	void on_abort_creation_mode();
	void on_execute_plugin();

	void update_menu();
	void optimize_current_view();

	Session *session;

	shared<hui::Panel> side_panel, bottom_panel;
	void set_side_panel(shared<hui::Panel> panel);
	void set_bottom_panel(shared<hui::Panel> panel);

	os::Timer timer;
};

#endif /* SRC_EDWARDWINDOW_H_ */
