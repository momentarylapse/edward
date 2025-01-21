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
class MultiView;
class Session;
class Painter;
class Data;

class Mode : obs::Node<VirtualBase> {
public:
	explicit Mode(Session* session);

	// this or a child
	virtual void enter_rec() {}
	virtual void leave_rec() {}

	// this exactly
	virtual void enter() {}
	virtual void leave() {}

	virtual void on_command(const string& id) {}

	virtual void on_mouse_move(const vec2&) {}
	virtual void on_mouse_leave(const vec2&) {}
	virtual void on_left_button_down(const vec2&) {}
	virtual void on_left_button_up(const vec2&) {}
	virtual void on_key_down(int key) {}
	virtual void on_draw_post(Painter*) {}

	virtual Renderer* create_renderer(SceneView* scene_view) { return nullptr; }

	Session* session = nullptr;
	MultiView* multi_view = nullptr;
	Data* generic_data = nullptr;
	Data* get_data() const { return generic_data; }
};



#endif //MODE_H
