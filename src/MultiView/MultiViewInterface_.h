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

class Data;

namespace MultiView{

class Window;
class SingleData;



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

struct Camera
{
	vector pos, ang;
	float zoom, radius;
	bool ignore_radius;
};

// multiview mask (data)
static const int FlagNone = 0;
static const int FlagSelect= 1;
static const int FlagDraw = 2;
static const int FlagIndex = 4;
static const int FlagMove = 8;

class MultiViewInterface : public Observable
{
public:
	MultiViewInterface();
	virtual ~MultiViewInterface();

	//virtual void SetViewStage(int *view_stage, bool allow_handle) = 0;
	virtual void SetData(int type, const DynamicArray &a, void *user_data, int mode) = 0;
	virtual void Reset() = 0;
	virtual void ResetView() = 0;
	virtual void SetViewBox(const vector &min, const vector &max) = 0;
	virtual void ResetData(Data *_data) = 0;

	vector virtual GetCursor3d() = 0;
	vector virtual GetCursor3d(const vector &depth_reference) = 0;

	virtual void ResetMouseAction() = 0;
	virtual void SetMouseAction(const string &name, int mode) = 0;


	bool wire_mode;
	bool grid_enabled;
	bool light_enabled;

	int view_stage;

	bool allow_rect;
	bool allow_mouse_actions;
	Camera cam;

	vector m, v;

	struct Selection
	{
		int set, index, type;
		vector point;
		void reset();
	};
	Selection hover, selection;
};

};

#endif /* MULTIVIEWINTERFACE_H_ */
