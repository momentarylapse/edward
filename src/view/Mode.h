//
// Created by Michael Ankele on 2025-01-20.
//

#ifndef MODE_H
#define MODE_H

#include <lib/base/base.h>
#include <lib/math/vec2.h>
#include <lib/pattern/Observable.h>

struct SceneView;
class Renderer;
struct RenderParams;
class MultiView;
class Session;
class Painter;
class Data;
class MultiViewWindow;
struct Hover;

namespace xhui {
	class Panel;
}

class Mode : public obs::Node<VirtualBase> {
public:
	explicit Mode(Session* session);

	obs::source out_redraw{this, "redraw"};

	// this or a child
	virtual void on_enter_rec() {}
	virtual void on_leave_rec() {}

	// this exactly
	virtual void on_enter() {}
	virtual void on_leave() {}

	virtual void on_command(const string& id) {}

	virtual void on_mouse_move(const vec2& m, const vec2& d) {}
	virtual void on_mouse_leave(const vec2&) {}
	virtual void on_left_button_down(const vec2&) {}
	virtual void on_left_button_up(const vec2&) {}
	virtual void on_key_down(int key) {}
	virtual void on_prepare_scene(const RenderParams& params) {}
	virtual void on_draw_win(const RenderParams& params, MultiViewWindow* win) {}
	virtual void on_draw_post(Painter*) {}
	virtual Mode* get_parent() { return nullptr; }

	void draw_info(Painter* p, const string& msg);

	Session* session = nullptr;
	MultiView* multi_view = nullptr;
	Data* generic_data = nullptr;
	Data* get_data() const { return generic_data; }

	xhui::Panel* dialog = nullptr;
};



#endif //MODE_H
