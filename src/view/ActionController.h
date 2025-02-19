/*
 * ActionController.h
 *
 *  Created on: 21.01.2014
 *      Author: michi
 */

#ifndef ACTIONCONTROLLER_H_
#define ACTIONCONTROLLER_H_

#include "../lib/base/base.h"
#include "../lib/base/pointer.h"
#include "../lib/image/color.h"
#include "../lib/math/mat4.h"
#include "../lib/math/vec3.h"
#include "../lib/pattern/Observable.h"
#include <y/graphics-fwd.h>

struct PolygonMesh;
class ActionMultiView;
class Data;
class Painter;
class MultiView;
class MultiViewWindow;
class Material;
struct RenderParams;
struct RenderViewData;

enum class MouseActionMode {
	SELECT,
	MOVE,
	ROTATE,
	ROTATE_2D,
	SCALE,
	SCALE_2D,
	MIRROR,
	ONCE,
};

struct MouseAction {
	MouseActionMode mode;
	bool active;
	bool locked;

	void reset() {
		//name = "";
		mode = MouseActionMode::SELECT;
		active = false;
		locked = false;
	}
	string name() const;
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

	bool visible = false;

	struct Manipulator {
		explicit Manipulator(MultiView* multi_view);
		void update(ActionController* ac);
		vec3 pos, pos0;
		float scale;
		owned_array<PolygonMesh> geo_show;
		owned_array<PolygonMesh> geo;
		owned_array<VertexBuffer> buf;
		owned_array<Material> materials;
		owned<Material> material_hover;
		mat4 geo_mat;
	} manipulator;
	vec3 m0;
	vec3 dv, dvp;
	vec3 param;
	mat4 mat;
	Constraint constraints;
	MultiView* multi_view;
	MouseAction action;
	ActionMultiView* cur_action = nullptr;
	Data* data;
	//void reset();
	//void draw(Window *win);
	void draw(const RenderParams& params, RenderViewData& rvd);
	void draw_post(Painter* p);
	void show(bool show);

	void update_manipulator();
	/*bool on_left_button_down(const vec2& m);
	void on_mouse_move(const vec2& m, const vec2& d);
	void on_left_button_up(const vec2& m);*/
	Constraint get_hover(MultiViewWindow* win, const vec2& m, vec3 &tp);
	bool in_use();
	void start_action(ActionMultiView* a, const vec3& m, Constraint constraints);
	void update_action(const vec2& m);
	void update_param(const vec3 &p);
	void end_action(bool set);
	bool is_selecting();


	static vec3 transform_ang(MultiViewWindow* w, const vec3& ang);
	static vec3 project_trans(Constraint mode, const vec3& v);
	static vec3 mirror(Constraint mode);


	static string constraint_name(Constraint c);



	struct GeoConfig {
		color col;
		Constraint constraint;
		int priority;
	};
	static const GeoConfig geo_config[];
	static bool geo_allow(int i, MultiViewWindow* win, const mat4& geo_mat);
};

#endif /* ACTIONCONTROLLER_H_ */
