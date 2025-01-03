/*
 * MultiView.h
 *
 *  Created on: 04.03.2012
 *      Author: michi
 */

#ifndef MULTIVIEW_H_
#define MULTIVIEW_H_

#include "../lib/base/base.h"
#include "../lib/base/pointer.h"
#include "../lib/math/vec3.h"
#include "../lib/math/vec2.h"
#include "../lib/math/quaternion.h"
#include "../lib/math/rect.h"
#include "../lib/pattern/Observable.h"
#include <y/graphics-fwd.h>

class Data;
class Session;
class DrawingHelper;
class Renderer;


namespace hui {
	class Menu;
}

namespace MultiView {

class MultiView;
class Window;
class SingleData;


enum {
	VIEW_RIGHT,
	VIEW_LEFT,
	VIEW_FRONT,
	VIEW_BACK,
	VIEW_TOP,
	VIEW_BOTTOM,
	VIEW_PERSPECTIVE,
	VIEW_2D,
	VIEW_ISOMETRIC
};


enum {
	ACTION_SELECT,
	ACTION_MOVE,
	ACTION_ROTATE,
	ACTION_ROTATE_2D,
	ACTION_SCALE,
	ACTION_SCALE_2D,
	ACTION_MIRROR,
	ACTION_ONCE,
};

struct Camera {
	vec3 pos;
	quaternion ang;
	float radius;
	bool ignore_radius;
	vec3 get_pos(bool allow_radius) const;
};


class ActionController;
class CameraController;

typedef std::function<float(const SingleData *p, Window *all_windows, const vec2 &m, vec3 &tp, float &z)> HoverDistanceFunction;

struct DataSet {
	int type;
	DynamicArray *data;
	bool selectable, drawable, movable, indexable;
	HoverDistanceFunction func_hover;
};




// multiview mask (data)
static const int FLAG_NONE = 0;
static const int FLAG_SELECT= 1;
static const int FLAG_DRAW = 2;
static const int FLAG_INDEX = 4;
static const int FLAG_MOVE = 8;




class MultiView : public obs::Node<VirtualBase> {
public:
	MultiView(Session *s, bool _mode3d);
	~MultiView() override;

	obs::source out_redraw{this, "redraw"};
	obs::source out_selection_changed{this, "selection-changed"};
	obs::source out_settings_changed{this, "settings-changed"};
	obs::source out_camera_changed{this, "camera-changed"};
	obs::source out_viewstage_changed{this, "viewstage-changed"};
	obs::source out_action_start{this, "action-start"};
	obs::source out_action_update{this, "action-update"};
	obs::source out_action_abort{this, "action-abort"};
	obs::source out_action_execute{this, "action-execute"};


	void on_mouse_move(const vec2& m);
	void on_mouse_wheel(const vec2& scroll);
	void on_mouse_enter(const vec2& m);
	void on_mouse_leave();
	void on_left_button_down(const vec2& m);
	void on_left_button_up(const vec2& m);
	void on_middle_button_down(const vec2& m);
	void on_middle_button_up(const vec2& m);
	void on_right_button_down(const vec2& m);
	void on_right_button_up(const vec2& m);
	void on_gesture_zoom_begin();
	void on_gesture_zoom(float factor);
	void on_key_down(int key_code);
	void on_key_up(int key_code);
	void on_command(const string &id);

	void on_update_menu();

	Session *session;
	Context *ctx;
	DrawingHelper *drawing_helper;
	bool mode3d;
	bool whole_window;
	rect area;

	bool wire_mode;
	bool grid_enabled;
	bool light_enabled;

	UniformBuffer *ubo_light;
	void set_light(Window *win, const vec3 &dir, const color &col, float harshness);

	bool snap_to_grid;

	int view_stage;

	bool allow_mouse_actions;
	Camera cam;

	vec2 m, v;
	float screen_scale;

	Window *active_win;
	Window *mouse_win;

	struct Selection {
		enum {
			HOVER_NONE,
			HOVER_WINDOW_DIVIDER_X,
			HOVER_WINDOW_DIVIDER_Y,
			HOVER_WINDOW_DIVIDER_CENTER,
			HOVER_WINDOW_LABEL,
			HOVER_ACTION_CONTROLLER,
			HOVER_CAMERA_CONTROLLER,
			HOVER_DATA
		};
		int meta;
		int set, index, type;
		SingleData *data;
		vec3 point;
		void reset();
	};
	Selection hover;

	void force_redraw();
	Renderer* renderer;

	void draw_mouse_pos();
	void toggle_whole_window();
	void toggle_grid();
	void toggle_light();
	void toggle_wire();
	void toggle_snap_to_grid();
	void cam_zoom(float factor, bool mouse_rel);
	void cam_move(const vec3 &dpos);
	void cam_move_pixel(Window *win, const vec3 &dir);
	void cam_rotate(const quaternion &dang, bool cam_center);
	void cam_rotate_pixel(const vec3 &dir, bool cam_center);
	void clear_data(Data *_data);
	void add_data(int type, const DynamicArray &a, int flags);
	void set_hover_func(int type, HoverDistanceFunction f);
	void set_view_stage(int *view_stage, bool allow_handle);
	void reset();
	void reset_view();
	void set_view_box(const vec3 &min, const vec3 &max);
	void set_allow_select(bool allow);
	void set_allow_action(bool allow);
	void push_settings();
	void pop_settings();
	struct Settings {
		bool allow_select;
		bool allow_action;
		string action_name;
		int action_mode;
		bool action_locked;
	};
	Array<Settings> settings_stack;

	void view_stage_push();
	void view_stage_pop();

	void select_all();
	void select_none();
	void invert_selection();
	bool has_selection();
	vec3 get_selection_center();

	void hold_cursor(bool holding);
	void start_selection_rect();
	void end_selection_rect();
	void update_mouse();

	void get_hover();
	enum class SelectionMode {
		SET,
		ADD,
		INVERT
	};
	void get_selected(SelectionMode mode = SelectionMode::SET);
	void select_all_in_rectangle(SelectionMode mode = SelectionMode::SET);
	bool hover_selected();
	bool has_selectable_data();
	void selection_changed_manually();

	string get_unit_by_zoom(vec3 &v);
	string format_length(float l);
	static vec3 snap_v2(const vec3 &v, float d);
	vec3 snap_v(const vec3 &v);
	float snap_f(float f);
	vec3 maybe_snap_v2(const vec3 &v, float d);
	vec3 maybe_snap_v(const vec3 &v);
	float maybe_snap_f(float f);

	vec3 get_cursor();
	vec3 get_cursor(const vec3 &depth_reference);

	enum class CoordinateMode {
		GLOBAL,
		CAMERA
	};
	CoordinateMode edit_coordinate_mode;
	void set_edit_coordinate_mode(CoordinateMode mode);


	Array<Window*> all_windows;
	Array<Window*> visible_windows;

	bool lbut, mbut, rbut;

	ActionController *action_con;
	void reset_mouse_action();
	void set_mouse_action(const string &name, int mode, bool locked);
	bool need_action_controller();
	CameraController *cam_con;

	Array<DataSet> data;
	bool allow_view_stage, allow_view_stage_handling;

	bool allow_infinite_scrolling;
	bool holding_cursor;
	float holding_x, holding_y;

	struct SelectionRect {
		bool active;
		int dist;
		vec2 pos0;
		void start_later(const vec2 &m);
		void end();
		rect get(const vec2 &m);
		void draw(DrawingHelper *drawing_helper, const vec2 &m);
	};
	SelectionRect sel_rect;
	bool allow_select;
	bool view_moving;


	int moving_win;
	vec3 moving_start, moving_dp;

	bool moving_cross_x;
	bool moving_cross_y;
	float window_partition_x;
	float window_partition_y;

	hui::Menu *menu;

	struct Message3d {
		string str;
		vec3 pos;
	};
	Array<Message3d> message3d;
	void add_message_3d(const string &str, const vec3 &pos);
	void reset_message_3d();


	float SPEED_MOVE;
	float SPEED_ZOOM_KEY;
	float SPEED_ZOOM_WHEEL;
	float HOVER_RADIUS;

	int MIN_MOUSE_MOVE_TO_INTERACT;
	float MOUSE_ROTATION_SPEED;
};

};

#endif /* MULTIVIEW_H_ */
