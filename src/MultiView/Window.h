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
class Camera;

class Window
{
public:
	Window(MultiView *view, int type);

	int type;
	matrix projection_matrix;
	matrix view_matrix;
	matrix pv_matrix;
	matrix ipv_matrix;
	rect dest;
	rect name_dest;
	MultiView *multi_view;
	quaternion local_ang;
	Camera *cam;
	float zoom();
	float get_grid_d();

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
