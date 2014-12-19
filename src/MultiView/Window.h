/*
 * Window.h
 *
 *  Created on: 21.01.2014
 *      Author: michi
 */

#ifndef MULTIVIEWWINDOW_H_
#define MULTIVIEWWINDOW_H_

#include "../lib/math/math.h"

namespace MultiView{

class MultiView;
class MultiViewImpl;
class Camera;

class Window
{
public:
	Window(MultiViewImpl *impl, int type);

	int type;
	matrix mat;
	rect dest;
	rect name_dest;
	quaternion ang;
	matrix projection;
	MultiView *multi_view;
	MultiViewImpl *impl;
	Camera *cam;

	void draw();
	void drawGrid();
	vector project(const vector &p);
	vector unproject(const vector &p);
	vector unproject(const vector &p, const vector &o);
	vector getDirection();
	vector getDirectionUp();
	vector getDirectionRight();
	void getMovingFrame(vector &dir, vector &up, vector &right);

	color getBackgroundColor();
};

};

#endif /* MULTIVIEWWINDOW_H_ */
