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

class ActionMultiView;
class Geometry;
class Data;

namespace MultiView{

class MultiView;
class MultiViewImpl;
class Window;


struct MouseAction
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
	ActionController(MultiViewImpl *impl);
	bool show;
	vector pos, pos0, m0;
	vector param;
	matrix mat;
	int mode;
	Array<Geometry*> geo_show;
	Array<Geometry*> geo;
	int mouse_over_geo;
	MultiViewImpl *multi_view;
	MouseAction action;
	ActionMultiView *cur_action;
	Data *data;
	void reset();
	void draw(Window *win);
	void drawParams();
	void disable();
	void enable();
	void update();
	bool leftButtonDown();
	void mouseMove();
	void leftButtonUp();
	bool isMouseOver(vector &tp);
	bool inUse();
	void startAction();
	void updateAction();
	void endAction(bool set);
	bool isSelecting();
};

};

#endif /* ACTIONCONTROLLER_H_ */
