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
	ACTION_NONE,
	ACTION_SELECT,
	ACTION_MOVE,
	ACTION_ROTATE,
	ACTION_ROTATE_2D,
	ACTION_SCALE,
	ACTION_SCALE_2D,
	ACTION_MIRROR,
	ACTION_ONCE,
	ACTION_SELECT_AND_MOVE,
};

struct Camera
{
	vector pos;
	quaternion ang;
	float zoom, radius;
	bool ignore_radius;
};

// multiview mask (data)
static const int FlagNone = 0;
static const int FlagSelect= 1;
static const int FlagDraw = 2;
static const int FlagIndex = 4;
static const int FlagMove = 8;

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

	virtual void ClearData(Data *data) = 0;
	virtual void AddData(int type, const DynamicArray &a, void *user_data, int flags) = 0;
	//virtual void SetViewStage(int *view_stage, bool allow_handle) = 0;
	virtual void Reset() = 0;
	virtual void ResetView() = 0;
	virtual void SetViewBox(const vector &min, const vector &max) = 0;

	vector virtual GetCursor3d() = 0;
	vector virtual GetCursor3d(const vector &depth_reference) = 0;
	virtual float GetGridD() = 0;

	virtual void ResetMouseAction() = 0;
	virtual void SetMouseAction(const string &name, int mode) = 0;


	virtual void AddMessage3d(const string &str, const vector &pos) = 0;
	virtual void ResetMessage3d() = 0;

	virtual void SetAllowRect(bool allow) = 0;
	virtual void SetAllowAction(bool allow) = 0;

	bool mode3d;
	bool whole_window;

	int light;

	bool wire_mode;
	bool grid_enabled;
	bool light_enabled;

	int view_stage;

	bool allow_rect;
	bool allow_mouse_actions;
	Camera cam;

	vector m, v;

	Window *active_win;
	Window *mouse_win;

	struct Selection
	{
		int set, index, type;
		vector point;
		void reset();
	};
	Selection hover;
};

};

#endif /* MULTIVIEW_H_ */
