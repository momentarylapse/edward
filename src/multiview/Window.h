/*
 * Window.h
 *
 *  Created on: 21.01.2014
 *      Author: michi
 */

#ifndef MULTIVIEWWINDOW_H_
#define MULTIVIEWWINDOW_H_

#include "../lib/math/mat4.h"
#include "../lib/math/vec3.h"
#include "../lib/math/rect.h"
#include "../lib/math/quaternion.h"

class color;
class DrawingHelper;

namespace nix {
	class Context;
	class Shader;
}

namespace MultiView{

class MultiView;
class Camera;

class Window {
public:
	Window(MultiView *view, int type);

	int type;
	mat4 projection_matrix;
	mat4 reflection_matrix;
	mat4 view_matrix;
	mat4 pv_matrix;
	mat4 ipv_matrix;
	rect dest;
	rect name_dest;
	nix::Context *gl;
	DrawingHelper *drawing_helper;
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
	vec3 active_grid_direction();

	vec3 project(const vec3 &p);
	vec3 unproject(const vec3 &p);
	vec3 unproject(const vec3 &p, const vec3 &o);

	void update_matrices();
	void set_projection_matrix();
	void set_projection_matrix_pixel();

	vec3 get_direction();
	vec3 get_edit_direction();
	void get_camera_frame(vec3 &dir, vec3 &up, vec3 &right);
	void get_active_grid_frame(vec3 &dir, vec3 &up, vec3 &right);
	void get_edit_frame(vec3 &dir, vec3 &up, vec3 &right);
	vec3 get_lighting_eye_pos();

	color get_background_color();
};

};

#endif /* MULTIVIEWWINDOW_H_ */
