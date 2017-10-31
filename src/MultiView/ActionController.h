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

namespace nix{
	class VertexBuffer;
}

namespace MultiView{

class MultiView;
class MultiViewImpl;
class Window;


struct MouseAction
{
	string name;
	int mode;
	bool active;
	bool locked;

	void reset()
	{
		name = "";
		mode = 0;
		active = false;
		locked = false;
	}
};


enum{
	ACTION_CONSTRAINTS_NONE,
	ACTION_CONSTRAINTS_X,
	ACTION_CONSTRAINTS_Y,
	ACTION_CONSTRAINTS_Z,
	ACTION_CONSTRAINTS_XY,
	ACTION_CONSTRAINTS_XZ,
	ACTION_CONSTRAINTS_YZ,
};

class ActionController
{
public:
	ActionController(MultiViewImpl *impl);
	bool visible;
	vector pos, pos0, m0;
	vector param;
	matrix mat;
	int constraints;
	Array<Geometry*> geo_show;
	Array<Geometry*> geo;
	Array<nix::VertexBuffer*> buf;
	matrix geo_mat;
	int mouse_over_geo;
	MultiViewImpl *multi_view;
	MouseAction action;
	ActionMultiView *cur_action;
	Data *data;
	void reset();
	void resetGeo();
	void draw(Window *win);
	void drawParams();
	void show(bool show);
	void update();
	bool leftButtonDown();
	void mouseMove();
	void leftButtonUp();
	bool isMouseOver(vector &tp);
	bool inUse();
	void startAction(int constraints);
	void updateAction();
	void endAction(bool set);
	bool isSelecting();
};

};

#endif /* ACTIONCONTROLLER_H_ */
