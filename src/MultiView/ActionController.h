/*
 * ActionController.h
 *
 *  Created on: 21.01.2014
 *      Author: michi
 */

#ifndef ACTIONCONTROLLER_H_
#define ACTIONCONTROLLER_H_

#include "../lib/base/base.h"
#include "../lib/math/math.h"

class ActionMultiView;
class Geometry;
class Data;

namespace nix {
	class VertexBuffer;
}

namespace MultiView {

class MultiView;
class Window;


struct MouseAction {
	string name;
	int mode;
	bool active;
	bool locked;

	void reset() {
		name = "";
		mode = 0;
		active = false;
		locked = false;
	}
};


enum {
	ACTION_CONSTRAINTS_NONE,
	ACTION_CONSTRAINTS_X,
	ACTION_CONSTRAINTS_Y,
	ACTION_CONSTRAINTS_Z,
	ACTION_CONSTRAINTS_XY,
	ACTION_CONSTRAINTS_XZ,
	ACTION_CONSTRAINTS_YZ,
};

class ActionController {
public:
	ActionController(MultiView *multi_view);
	~ActionController();
	bool visible;
	vector pos, pos0, m0;
	vector param;
	matrix mat;
	int constraints;
	Array<Geometry*> geo_show;
	Array<Geometry*> geo;
	Array<nix::VertexBuffer*> buf;
	matrix geo_mat;
	int mouse_over_constraint;
	MultiView *multi_view;
	Window *active_win;
	MouseAction action;
	ActionMultiView *cur_action;
	Data *data;
	void reset();
	void delete_geo();
	void draw(Window *win);
	void draw_post();
	void show(bool show);
	void update();
	bool on_left_button_down();
	void on_mouse_move();
	void on_left_button_up();
	bool is_mouse_over(vector &tp);
	bool in_use();
	void start_action(Window *active_win, const vector &m, int constraints);
	void update_action();
	void update_param(const vector &p);
	void end_action(bool set);
	bool is_selecting();
};

};

#endif /* ACTIONCONTROLLER_H_ */
