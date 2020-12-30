/*
 * CameraController.cpp
 *
 *  Created on: 26.01.2014
 *      Author: michi
 */

#include "CameraController.h"
#include "MultiView.h"
#include "Window.h"
#include "DrawingHelper.h"
#include "../lib/nix/nix.h"
#include "../Edward.h"

namespace MultiView{

const int CC_RADIUS = 40;
const int CC_MARGIN = 200;
const int CC_BORDER = 6;

const color ColorBackground = color(0.5f, 1, 1, 1);
const color ColorIcon = color(0.5f, 0.2f, 0.2f, 0.8f);
const color ColorIconHover = color(0.7f, 0.4f, 0.4f, 1);

CameraController::CameraController(MultiView *_view) {
	view = _view;
	show = true;
	tex_bg = nix::LoadTexture(app->directory_static << "icons/toolbar/multiview/bg.png");
	tex_move = nix::LoadTexture(app->directory_static << "icons/toolbar/multiview/move.png");
	tex_rotate= nix::LoadTexture(app->directory_static << "icons/toolbar/multiview/rotate.png");
	tex_zoom = nix::LoadTexture(app->directory_static << "icons/toolbar/multiview/zoom.png");
	//controllers.resize(4);
}

CameraController::~CameraController() {
}


CameraController::Controller::Controller() {
	win = nullptr;
	moving = false;
	rotating = false;
	zooming = false;
}

void CameraController::Controller::set(Window *w) {
	win = w;
	float margin = min(100.0f, w->dest.width() / 10.0f);
	r = rect(w->dest.x2 - margin - 3 * CC_RADIUS - 4 * CC_BORDER,
			w->dest.x2 - margin,
			w->dest.y1 + margin - CC_BORDER,
			w->dest.y1 + margin + CC_RADIUS + CC_BORDER);
	r_move = rect(w->dest.x2 - margin - 3 * CC_RADIUS - 3 * CC_BORDER,
				w->dest.x2 - margin - 2 * CC_RADIUS - 3 * CC_BORDER,
				w->dest.y1 + margin,
				w->dest.y1 + margin + CC_RADIUS);
	r_rotate = rect(w->dest.x2 - margin - 2 * CC_RADIUS - 2 * CC_BORDER,
				w->dest.x2 - margin - CC_RADIUS - 2 * CC_BORDER,
				w->dest.y1 + margin,
				w->dest.y1 + margin + CC_RADIUS);
	r_zoom = rect(w->dest.x2 - margin - CC_RADIUS - CC_BORDER,
				w->dest.x2 - margin - CC_BORDER,
				w->dest.y1 + margin,
				w->dest.y1 + margin + CC_RADIUS);
}

bool CameraController::Controller::hover(float mx, float my) {
	return r.inside(mx, my);
}

void CameraController::update_rects() {
	controllers.resize(view->visible_windows.num);
	foreachi (auto *w, view->visible_windows, i)
		controllers[i].set(w);

	r2 = rect(nix::target_width - CC_RADIUS / 2 - CC_BORDER,
	          nix::target_width + CC_RADIUS / 2 + CC_BORDER,
	          nix::target_height / 2 - CC_RADIUS / 2 - CC_BORDER,
	          nix::target_height / 2 + CC_RADIUS / 2 + CC_BORDER);
	r_show = rect(nix::target_width - CC_RADIUS / 2,
	              nix::target_width + CC_RADIUS / 2,
	              nix::target_height / 2 - CC_RADIUS / 2,
	              nix::target_height / 2 + CC_RADIUS / 2);
}

bool CameraController::is_mouse_over() {
	update_rects();
	if (show) {
		for (auto &c: controllers)
			if (c.hover(view->m.x, view->m.y))
				return true;
	}

	return r2.inside(view->m.x, view->m.y);
}

void CameraController::on_left_button_down() {
	update_rects();
	if (r_show.inside(view->m.x, view->m.y))
		show = !show;
	for (auto &c: controllers) {
		c.moving = c.r_move.inside(view->m.x, view->m.y);
		c.rotating = c.r_rotate.inside(view->m.x, view->m.y);
		c.zooming = c.r_zoom.inside(view->m.x, view->m.y);
	}
}

void CameraController::on_left_button_up() {
	for (auto &c: controllers) {
		c.moving = c.rotating = c.zooming = false;
	}
}

void CameraController::on_mouse_move() {
	for (auto &c: controllers) {
		if (c.moving)
			view->cam_move_pixel(c.win, vector(view->v.x, view->v.y, 0));
		if (c.rotating)
			view->cam_rotate_pixel(view->v, false);
		if (c.zooming)
			view->cam_zoom(pow(1.007f, view->v.y), false);
	}
}

void CameraController::on_mouse_wheel() {

	hui::Event *e = hui::GetEvent();

	// mouse wheel -> zoom
	if (e->scroll_y > 0)
		view->cam_zoom(view->SPEED_ZOOM_WHEEL, view->mouse_win->type != VIEW_PERSPECTIVE);
	if (e->scroll_y < 0)
		view->cam_zoom(1.0f / view->SPEED_ZOOM_WHEEL, view->mouse_win->type != VIEW_PERSPECTIVE);
}

void CameraController::draw_icon(const rect &rr, nix::Texture *tex, bool active) {
	nix::SetTexture(tex_bg);
	if (active or rr.inside(view->m.x, view->m.y))
		set_color(ColorIconHover);
	else
		set_color(ColorIcon);
	draw_2d(rect::ID, rr, 0);
	if (tex) {
		nix::SetTexture(tex);
		set_color(White);
		draw_2d(rect::ID, rr, 0);
	}
}

void CameraController::draw() {
	update_rects();
	nix::SetAlpha(ALPHA_MATERIAL);

	// show/hide button
	set_color(ColorBackground);
	nix::SetTexture(tex_bg);
	nix::SetShader(nix::default_shader_2d);
	draw_2d(rect::ID, r2, 0);
	draw_icon(r_show, NULL, false);

	if (show) {

		for (auto &c: controllers) {
		// elongated background
		set_color(ColorBackground);
		nix::SetTexture(tex_bg);
		draw_2d(rect(0, 0.5f, 0, 1), rect(c.r.x1, c.r.x1 + CC_RADIUS/2 + CC_BORDER, c.r.y1, c.r.y2), 0);
		draw_2d(rect(0.5f, 0.5f, 0, 1), rect(c.r.x1 + CC_RADIUS/2 + CC_BORDER, c.r.x2 - CC_RADIUS/2 - CC_BORDER, c.r.y1, c.r.y2), 0);
		draw_2d(rect(0.5f, 1, 0, 1), rect(c.r.x2 - CC_RADIUS/2 - CC_BORDER, c.r.x2, c.r.y1, c.r.y2), 0);

		// icons
		draw_icon(c.r_move, tex_move, c.moving);
		draw_icon(c.r_rotate, tex_rotate, c.rotating);
		draw_icon(c.r_zoom, tex_zoom, c.zooming);
		}
	}
	nix::SetTexture(NULL);
	nix::SetAlpha(ALPHA_NONE);
}

bool CameraController::in_use() {
	for (auto &c: controllers)
		if (c.moving or c.rotating or c.zooming)
			return true;
	return false;
}

};
