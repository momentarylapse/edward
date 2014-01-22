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

class MultiView;
class MultiViewWindow;
class ActionMultiView;
class Geometry;


struct MultiViewMouseAction
{
	string name;
	int mode;
	bool active;

	void reset()
	{
		name = "";
		mode = 0;
		active = false;
	}
};

class ActionController
{
public:
	bool show;
	vector pos, pos0, m0;
	vector param;
	matrix mat;
	int mode;
	Array<Geometry*> geo_show;
	Array<Geometry*> geo;
	int mouse_over_geo;
	MultiView *multi_view;
	MultiViewMouseAction action;
	ActionMultiView *cur_action;
	void reset();
	void Draw(MultiViewWindow *win);
	void DrawParams();
	void Disable();
	void Enable();
	void Update();
	bool LeftButtonDown();
	void MouseMove();
	void LeftButtonUp();
	bool IsMouseOver(vector &tp);
	bool InUse();
	void StartAction();
	void UpdateAction();
	void EndAction(bool set);
};

#endif /* ACTIONCONTROLLER_H_ */
