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

class Window {
public:
	Window(MultiView *view, int type);

	int type;
	matrix projection_matrix;
	matrix reflection_matrix;
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
	void draw_grid();
	void draw_data_points();
	void draw_header();

	vector project(const vector &p);
	vector unproject(const vector &p);
	vector unproject(const vector &p, const vector &o);

	void update_matrices();
	void set_projection_matrix();
	void set_projection_matrix_pixel();

	vector get_direction();
	vector get_direction_up();
	vector get_direction_right();
	void get_moving_frame(vector &dir, vector &up, vector &right);

	color get_background_color();
};

};

#endif /* MULTIVIEWWINDOW_H_ */
