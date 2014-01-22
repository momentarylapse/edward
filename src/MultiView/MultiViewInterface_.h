/*
 * MultiViewInterface.h
 *
 *  Created on: 21.01.2014
 *      Author: michi
 */

#ifndef MULTIVIEWINTERFACE_H_
#define MULTIVIEWINTERFACE_H_


#include "../Stuff/Observable.h"
#include "../lib/math/math.h"

class MultiViewWindow;
class Data;

// "des Pudels Kern", don't change!!!!!!!
class MultiViewSingleData
{
public:
	MultiViewSingleData();
	int view_stage;
	bool is_selected, m_delta, m_old, is_special;
	vector pos;
};

typedef bool t_is_mouse_over_func(int index, void *user_data, MultiViewWindow *win, vector &tp);
typedef bool t_is_in_rect_func(int index, void *user_data, MultiViewWindow *win, rect *r);



enum
{
	ViewRight,
	ViewLeft,
	ViewFront,
	ViewBack,
	ViewTop,
	ViewBottom,
	ViewPerspective,
	View2D,
	ViewIsometric
};


struct MultiViewCamera
{
	vector pos, ang;
	float zoom, radius;
	bool ignore_radius;
};

class MultiViewInterface : public Observable
{
public:
	MultiViewInterface();
	virtual ~MultiViewInterface();

	//virtual void SetViewStage(int *view_stage, bool allow_handle) = 0;
	virtual void SetData(int type, const DynamicArray &a, void *user_data, int mode, t_is_mouse_over_func *is_mouse_over_func, t_is_in_rect_func *is_in_rect_func) = 0;
	virtual void Reset() = 0;
	virtual void ResetView() = 0;
	virtual void SetViewBox(const vector &min, const vector &max) = 0;
	virtual void ResetData(Data *_data) = 0;

	vector virtual GetCursor3d() = 0;
	vector virtual GetCursor3d(const vector &depth_reference) = 0;

	enum{
		ActionNone,
		ActionSelect,
		ActionMove,
		ActionRotate,
		ActionRotate2d,
		ActionScale,
		ActionScale2d,
		ActionMirror,
		ActionOnce
	};
	virtual void ResetMouseAction() = 0;
	virtual void SetMouseAction(const string &name, int mode) = 0;


	// multiview mask (data)
	static const int FlagNone = 0;
	static const int FlagSelect= 1;
	static const int FlagDraw = 2;
	static const int FlagIndex = 4;
	static const int FlagMove = 8;


	bool wire_mode;
	bool grid_enabled;
	bool light_enabled;

	int view_stage;

	bool allow_rect;
	bool allow_mouse_actions;
	MultiViewCamera cam;

	vector m, v;

	struct Selection
	{
		int set, index, type;
		vector point;
		void reset();
	};
	Selection hover, selection;
};

#endif /* MULTIVIEWINTERFACE_H_ */
