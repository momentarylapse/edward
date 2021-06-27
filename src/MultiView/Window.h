/*
 * Window.h
 *
 *  Created on: 21.01.2014
 *      Author: michi
 */

#ifndef MULTIVIEWWINDOW_H_
#define MULTIVIEWWINDOW_H_

#include "../lib/math/matrix.h"
#include "../lib/math/vector.h"
#include "../lib/math/rect.h"
#include "../lib/math/quaternion.h"

class color;

namespace nix {
	class Shader;
}

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

	void set_shader(nix::Shader *s, int num_lights = 1);

	int active_grid();
	vector active_grid_direction();

	vector project(const vector &p);
	vector unproject(const vector &p);
	vector unproject(const vector &p, const vector &o);

	void update_matrices();
	void set_projection_matrix();
	void set_projection_matrix_pixel();

	vector get_direction();
	vector get_edit_direction();
	void get_camera_frame(vector &dir, vector &up, vector &right);
	void get_active_grid_frame(vector &dir, vector &up, vector &right);
	void get_edit_frame(vector &dir, vector &up, vector &right);
	vector get_lighting_eye_pos();

	color get_background_color();
};

};

#endif /* MULTIVIEWWINDOW_H_ */
