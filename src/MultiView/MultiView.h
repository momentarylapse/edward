/*
 * MultiView.h
 *
 *  Created on: 04.03.2012
 *      Author: michi
 */

#ifndef MULTIVIEW_H_
#define MULTIVIEW_H_

#include "../lib/file/file.h"
#include "../lib/nix/nix.h"
#include "../Data/Data.h"

class Data;
class Observable;

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

// multiview mask (data)
static const int FLAG_NONE = 0;
static const int FLAG_SELECT= 1;
static const int FLAG_DRAW = 2;
static const int FLAG_INDEX = 4;
static const int FLAG_MOVE = 8;

class MultiView : public Observable
{
public:
	MultiView(bool _mode3d);
	virtual ~MultiView();

	static const string MESSAGE_UPDATE;
	static const string MESSAGE_SELECTION_CHANGE;
	static const string MESSAGE_SETTINGS_CHANGE;
	static const string MESSAGE_ACTION_START;
	static const string MESSAGE_ACTION_UPDATE;
	static const string MESSAGE_ACTION_ABORT;
	static const string MESSAGE_ACTION_EXECUTE;

	virtual void clearData(Data *data) = 0;
	virtual void addData(int type, const DynamicArray &a, void *user_data, int flags) = 0;
	//virtual void SetViewStage(int *view_stage, bool allow_handle) = 0;
	virtual void reset() = 0;
	virtual void resetView() = 0;
	virtual void setViewBox(const vector &min, const vector &max) = 0;

	vector virtual getCursor3d() = 0;
	vector virtual getCursor3d(const vector &depth_reference) = 0;

	virtual void resetMouseAction() = 0;
	virtual void setMouseAction(const string &name, int mode, bool locked) = 0;
	//virtual int getMouseActionMode() = 0;


	virtual void addMessage3d(const string &str, const vector &pos) = 0;
	virtual void resetMessage3d() = 0;

	virtual void pushSettings() = 0;
	virtual void popSettings() = 0;

	virtual void setAllowAction(bool allow) = 0;
	virtual void setAllowSelect(bool allow) = 0;

	bool mode3d;
	bool whole_window;

	int light;

	bool wire_mode;
	bool grid_enabled;
	bool light_enabled;

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
};

};

#endif /* MULTIVIEW_H_ */
