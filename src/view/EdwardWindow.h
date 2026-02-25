//
// Created by michi on 19.01.25.
//

#pragma once

#include "lib/xhui/xhui.h"
#include "lib/pattern/Observable.h"

namespace xhui {
	class MenuBar;
	class Toolbar;
}
class Session;
class Mode;
class DocumentSwitcher;
class ProjectPanel;

class EdwardWindow : public obs::Node<xhui::Window> {
public:
	owned<Session> session;
	Array<string> args;

	obs::sink in_redraw;
	obs::sink in_data_selection_changed;
	obs::sink in_data_changed;
	obs::sink in_action_failed;
	obs::sink in_saved;

	xhui::MenuBar* menu_bar;
	xhui::Toolbar* tool_bar;
	xhui::MenuBar* project_menu_bar;

	explicit EdwardWindow(xfer<Session> session);

	void on_key_down(int key) override;
	void on_key_up(int key_code) override;

	void update_menu();

	Mode* cur_mode() const;

	DocumentSwitcher* switcher = nullptr;
	ProjectPanel* project_panel = nullptr;
};

