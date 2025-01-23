/*
 * ActionController.h
 *
 *  Created on: 21.01.2014
 *      Author: michi
 */

#ifndef ACTIONCONTROLLER_H_
#define ACTIONCONTROLLER_H_

#include "../lib/base/base.h"
#include "../lib/image/color.h"
#include "../lib/math/mat4.h"
#include "../lib/math/vec3.h"
#include "../lib/pattern/Observable.h"
#include <y/graphics-fwd.h>

class ActionMultiView;
class Geometry;
class Data;
class Painter;
class MultiView;
//class Window;
class Material;
struct RenderParams;
struct RenderViewData;


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


class ActionController : public obs::Node<VirtualBase> {
public:
	explicit ActionController(MultiView *multi_view);
	~ActionController() override;

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
	vec3 pos, pos0, m0;
	vec3 dv, dvp;
	vec3 param;
	mat4 mat;
	Constraint constraints;
	Array<Geometry*> geo_show;
	Array<Geometry*> geo;
	Array<VertexBuffer*> buf;
	Material* material;
	mat4 geo_mat;
	Constraint hover_constraint;
	MultiView *multi_view;
//	Window *active_win;
	MouseAction action;
	ActionMultiView *cur_action;
	Data *data;
	void reset();
	void delete_geo();
	//void draw(Window *win);
	void draw(const RenderParams& params, RenderViewData& rvd);
	void draw_post();
	void show(bool show);
	void update();
	bool on_left_button_down();
	void on_mouse_move();
	void on_left_button_up();
	Constraint get_hover(vec3 &tp);
	bool in_use();
	//void start_action(Window *active_win, const vec3 &m, Constraint constraints);
	void update_action();
	void update_param(const vec3 &p);
	void end_action(bool set);
	bool is_selecting();


	//static vec3 transform_ang(Window *w, const vec3 &ang);
	static vec3 project_trans(Constraint mode, const vec3 &v);
	static vec3 mirror(Constraint mode);


	static string constraint_name(Constraint c);

	static string action_name(int a);



	struct ACGeoConfig {
		color col;
		Constraint constraint;
		int priority;
	};
	static const ACGeoConfig ac_geo_config[];
	static bool geo_allow(int i, const mat4& proj, const mat4& geo_mat);
};

#endif /* ACTIONCONTROLLER_H_ */
