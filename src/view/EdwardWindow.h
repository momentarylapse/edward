//
// Created by michi on 19.01.25.
//

#ifndef EDWARDWINDOW_H
#define EDWARDWINDOW_H

#include "lib/xhui/xhui.h"
#include "lib/pattern/Observable.h"

namespace xhui {
	class Toolbar;
}
namespace yrenderer {
	class XhuiRenderer;
}
class MultiViewRenderer;
class Session;

class EdwardWindow : public obs::Node<xhui::Window> {
public:
	owned<Session> session;
	yrenderer::XhuiRenderer* renderer = nullptr;
	MultiViewRenderer* multi_view_renderer = nullptr;
	Array<string> args;

	obs::sink in_redraw;
	obs::sink in_data_selection_changed;
	obs::sink in_data_changed;
	obs::sink in_action_failed;
	obs::sink in_saved;

	xhui::Toolbar* toolbar;

	explicit EdwardWindow(xfer<Session> session);

	void on_key_down(int key) override;

	void update_menu();
};


#endif //EDWARDWINDOW_H
