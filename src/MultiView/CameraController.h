/*
 * CameraController.h
 *
 *  Created on: 26.01.2014
 *      Author: michi
 */

#ifndef CAMERACONTROLLER_H_
#define CAMERACONTROLLER_H_


#include "../lib/math/rect.h"

namespace nix {
	class Texture;
};

namespace MultiView {

class MultiView;
class Window;

class CameraController {
public:
	CameraController(MultiView *view);
	~CameraController();

	bool is_mouse_over();
	void on_left_button_down();
	void on_left_button_up();
	void on_mouse_wheel();
	void on_mouse_move();
	void update_rects();

	void draw_icon(const rect &r, nix::Texture *tex, bool active);
	void draw();
	bool in_use();

	bool show;

	struct Controller {
		Controller();
		Window* win;
		rect r, r_move, r_rotate, r_zoom;
		bool moving, rotating, zooming;
		void set(Window *w);
		bool hover(float mx, float my);
		void on_left_button_down(float mx, float my);
	};
	Array<Controller> controllers;

	rect r2, r_show;
	MultiView *view;
	nix::Texture *tex_bg;
	nix::Texture *tex_move, *tex_rotate, *tex_zoom;
};

};

#endif /* CAMERACONTROLLER_H_ */
