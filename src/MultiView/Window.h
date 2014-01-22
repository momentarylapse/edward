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
	Window(MultiViewImpl *impl);

	int type;
	matrix mat;
	rect dest;
	rect name_dest;
	vector ang;
	matrix projection;
	MultiView *multi_view;
	MultiViewImpl *impl;
	Camera *cam;

	void Draw();
	void DrawGrid();
	vector Project(const vector &p);
	vector Unproject(const vector &p);
	vector Unproject(const vector &p, const vector &o);
	vector GetDirection();
	vector GetDirectionUp();
	vector GetDirectionRight();
	void GetMovingFrame(vector &dir, vector &up, vector &right);
};

};

#endif /* MULTIVIEWWINDOW_H_ */
