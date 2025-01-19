//
// Created by michi on 19.01.25.
//

#ifndef EDWARDWINDOW_H
#define EDWARDWINDOW_H

#include "lib/xhui/xhui.h"

class XhuiRenderer;
class MultiViewRenderer;
class Session;

class EdwardWindow : public xhui::Window {
public:
	Session* session;
	XhuiRenderer* renderer = nullptr;
	MultiViewRenderer* multi_view_renderer = nullptr;
	Array<string> args;

	explicit EdwardWindow(Session* session);

	void on_mouse_move(const vec2& m, const vec2& d) override;
	void on_mouse_wheel(const vec2& d) override;
	void on_key_down(int key) override;
};


#endif //EDWARDWINDOW_H
