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

namespace nix{
	class Shader;
}

namespace MultiView{

class MultiView;
class Window;
class SingleData;


enum
{
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


enum{
	ACTION_SELECT,
	ACTION_MOVE,
	ACTION_ROTATE,
	ACTION_ROTATE_2D,
	ACTION_SCALE,
	ACTION_SCALE_2D,
	ACTION_MIRROR,
	ACTION_ONCE,
};

struct Camera
{
	vector pos;
	quaternion ang;
	float radius;
	bool ignore_radius;
};


class ActionController;
class CameraController;

typedef bool HoverFunction(const SingleData *p, Window *win, vector &m, vector &tp, float &z, void *user_data);

struct DataSet
{
	int type;
	DynamicArray *data;
	bool selectable, drawable, movable, indexable;
	void *user_data;
	HoverFunction *func_hover;
};



extern color ColorBackGround;
extern color ColorBackGroundSelected;
extern color ColorGrid;
extern color ColorText;
extern color ColorWindowType;
extern color ColorPoint;
extern color ColorPointSelected;
extern color ColorPointSpecial;
extern color ColorWindowSeparator;
extern color ColorSelectionRect;
extern color ColorSelectionRectBoundary;


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


void set_wide_lines(float width);

class MultiView : public Observable
{
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

	struct Selection
	{
		enum{
			HOVER_NONE,
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


	static color ColorBackGround;
	static color ColorBackGroundSelected;
	static color ColorGrid;
	static color ColorText;
	static color ColorWindowType;
	static color ColorPoint;
	static color ColorPointSelected;
	static color ColorPointSpecial;
	static color ColorWindowSeparator;
	static color ColorSelectionRect;
	static color ColorSelectionRectBoundary;
	static color ColorCreation;
	static color ColorCreationLine;

	void on_draw();
	void drawMousePos();
	void toggleWholeWindow();
	void toggleGrid();
	void toggleLight();
	void toggleWire();
	void toggleSnapToGrid();
	void camZoom(float factor, bool mouse_rel);
	void camMove(const vector &dir);
	void camRotate(const vector &dir, bool cam_center);
	void setMode(int mode);
	void clearData(Data *_data);
	void addData(int type, const DynamicArray &a, void *user_data, int flags);
	void set_hover_func(int type, HoverFunction *f);
	void SetViewStage(int *view_stage, bool allow_handle);
	void reset();
	void resetView();
	void setViewBox(const vector &min, const vector &max);
	void setAllowSelect(bool allow);
	void setAllowAction(bool allow);
	void pushSettings();
	void popSettings();
	struct Settings
	{
		bool allow_select;
		bool allow_action;
		string action_name;
		int action_mode;
		bool action_locked;
	};
	Array<Settings> settings_stack;

	void viewStagePush();
	void viewStagePop();

	void selectAll();
	void selectNone();
	void invertSelection();
	bool hasSelection();
	vector getSelectionCenter();

	void holdCursor(bool holding);
	void startRect();
	void endRect();
	void updateMouse();

	void getHover();
	void unselectAll();
	enum{
		SELECT_SET,
		SELECT_ADD,
		SELECT_INVERT
	};
	void getSelected(int mode = SELECT_SET);
	void selectAllInRectangle(int mode = SELECT_SET);
	bool hoverSelected();
	bool hasSelectableData();

	string getScaleByZoom(vector &v);

	vector getCursor3d();
	vector getCursor3d(const vector &depth_reference);


	Array<Window*> win;

	bool lbut, mbut, rbut;

	ActionController *action_con;
	void resetMouseAction();
	void setMouseAction(const string &name, int mode, bool locked);
	bool needActionController();
	CameraController *cam_con;

	Array<DataSet> data;
	bool allow_view_stage, allow_view_stage_handling;

	bool allow_infinite_scrolling;
	bool holding_cursor;
	float holding_x, holding_y;

	struct SelectionRect
	{
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

	hui::Menu *menu;

	struct Message3d
	{
		string str;
		vector pos;
	};
	Array<Message3d> message3d;
	void addMessage3d(const string &str, const vector &pos);
	void resetMessage3d();


	float SPEED_MOVE;
	float SPEED_ZOOM_KEY;
	float SPEED_ZOOM_WHEEL;

	int MIN_MOUSE_MOVE_TO_INTERACT;
	float MOUSE_ROTATION_SPEED;



	int POINT_RADIUS;
	int POINT_RADIUS_HOVER;
};

};

#endif /* MULTIVIEW_H_ */
