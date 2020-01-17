/*
 * MultiView.h
 *
 *  Created on: 04.03.2012
 *      Author: michi
 */

#ifndef MULTIVIEW_H_
#define MULTIVIEW_H_

#include "../lib/base/base.h"
#include "../Stuff/Observable.h"

class Data;

namespace nix {
	class Shader;
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
	vector pos;
	quaternion ang;
	float radius;
	bool ignore_radius;
};


class ActionController;
class CameraController;

typedef bool HoverFunction(const SingleData *p, Window *win, vector &m, vector &tp, float &z, void *user_data);

struct DataSet {
	int type;
	DynamicArray *data;
	bool selectable, drawable, movable, indexable;
	void *user_data;
	HoverFunction *func_hover;
};




// multiview mask (data)
static const int FLAG_NONE = 0;
static const int FLAG_SELECT= 1;
static const int FLAG_DRAW = 2;
static const int FLAG_INDEX = 4;
static const int FLAG_MOVE = 8;



extern nix::Shader *shader_selection;
extern nix::Shader *shader_lines_3d;
extern nix::Shader *shader_lines_3d_colored;
extern nix::Shader *shader_lines_3d_colored_wide;


class MultiView : public Observable {
public:
	MultiView(bool _mode3d);
	virtual ~MultiView();

	static const string MESSAGE_UPDATE;
	static const string MESSAGE_SELECTION_CHANGE;
	static const string MESSAGE_SETTINGS_CHANGE;
	static const string MESSAGE_CAMERA_CHANGE;
	static const string MESSAGE_VIEWSTAGE_CHANGE;
	static const string MESSAGE_ACTION_START;
	static const string MESSAGE_ACTION_UPDATE;
	static const string MESSAGE_ACTION_ABORT;
	static const string MESSAGE_ACTION_EXECUTE;


	void on_mouse_move();
	void on_mouse_wheel();
	void on_mouse_enter();
	void on_mouse_leave();
	void on_left_button_down();
	void on_left_button_up();
	void on_middle_button_down();
	void on_middle_button_up();
	void on_right_button_down();
	void on_right_button_up();
	void on_key_down(int key_code);
	void on_key_up(int key_code);
	void on_command(const string &id);

	void on_update_menu();

	bool mode3d;
	bool whole_window;
	rect area;

	int light;

	bool wire_mode;
	bool grid_enabled;
	bool light_enabled;

	bool snap_to_grid;

	int view_stage;

	bool allow_mouse_actions;
	Camera cam;

	vector m, v;

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
		vector point;
		void reset();
	};
	Selection hover;

	void force_redraw();

	void on_draw();
	void draw_mouse_pos();
	void toggle_whole_window();
	void toggle_grid();
	void toggle_light();
	void toggle_wire();
	void toggle_snap_to_grid();
	void cam_zoom(float factor, bool mouse_rel);
	void cam_move(const vector &dpos);
	void cam_move_pixel(const vector &dir);
	void cam_rotate(const quaternion &dang, bool cam_center);
	void cam_rotate_pixel(const vector &dir, bool cam_center);
	void clear_data(Data *_data);
	void add_data(int type, const DynamicArray &a, void *user_data, int flags);
	void set_hover_func(int type, HoverFunction *f);
	void set_view_stage(int *view_stage, bool allow_handle);
	void reset();
	void reset_view();
	void set_view_box(const vector &min, const vector &max);
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
	vector get_selection_center();

	void hold_cursor(bool holding);
	void start_selection_rect();
	void end_selection_rect();
	void update_mouse();

	void get_hover();
	enum {
		SELECT_SET,
		SELECT_ADD,
		SELECT_INVERT
	};
	void get_selected(int mode = SELECT_SET);
	void select_all_in_rectangle(int mode = SELECT_SET);
	bool hover_selected();
	bool has_selectable_data();

	string get_unit_by_zoom(vector &v);
	string format_length(float l);
	static vector snap_v2(const vector &v, float d);
	vector snap_v(const vector &v);
	float snap_f(float f);
	vector maybe_snap_v2(const vector &v, float d);
	vector maybe_snap_v(const vector &v);
	float maybe_snap_f(float f);

	vector get_cursor();
	vector get_cursor(const vector &depth_reference);


	Array<Window*> win;

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
		vector pos0;
		void start_later(const vector &m);
		void end();
		rect get(const vector &m);
		void draw(const vector &m);
	};
	SelectionRect sel_rect;
	bool allow_select;
	bool view_moving;


	int moving_win;
	vector moving_start, moving_dp;

	bool moving_cross_x;
	bool moving_cross_y;
	float window_partition_x;
	float window_partition_y;

	hui::Menu *menu;

	struct Message3d {
		string str;
		vector pos;
	};
	Array<Message3d> message3d;
	void add_message_3d(const string &str, const vector &pos);
	void reset_message_3d();


	float SPEED_MOVE;
	float SPEED_ZOOM_KEY;
	float SPEED_ZOOM_WHEEL;

	int MIN_MOUSE_MOVE_TO_INTERACT;
	float MOUSE_ROTATION_SPEED;
};

};

#endif /* MULTIVIEW_H_ */
