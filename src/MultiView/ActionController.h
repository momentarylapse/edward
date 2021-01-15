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


class ActionController {
public:
	ActionController(MultiView *multi_view);
	~ActionController();

	enum class Constraint {
		UNDEFINED = -1,
		FREE,
		X,
		Y,
		Z,
		NEG_X,
		NEG_Y,
		NEG_Z,
		XY,
		XZ,
		YZ,
	};

	bool visible;
	vector pos, pos0, m0;
	vector dv, dvp;
	vector param;
	matrix mat;
	Constraint constraints;
	Array<Geometry*> geo_show;
	Array<Geometry*> geo;
	Array<nix::VertexBuffer*> buf;
	matrix geo_mat;
	Constraint hover_constraint;
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
	Constraint get_hover(vector &tp);
	bool in_use();
	void start_action(Window *active_win, const vector &m, Constraint constraints);
	void update_action();
	void update_param(const vector &p);
	void end_action(bool set);
	bool is_selecting();


	static vector transform_ang(Window *w, const vector &ang);
	static vector project_trans(Constraint mode, const vector &v);
	static vector mirror(Constraint mode);


	static string constraint_name(Constraint c);

	static string action_name(int a);



	struct ACGeoConfig {
		color col;
		Constraint constraint;
		int priority;
	};
	static const ACGeoConfig ac_geo_config[];
	static bool geo_allow(int i, Window *win, const matrix &geo_mat);
};

};

#endif /* ACTIONCONTROLLER_H_ */
