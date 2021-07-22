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
#include "MouseWrapper.h"
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
	tex_bg = nix::Texture::load(app->directory_static << "icons/toolbar/multiview/bg.png");
	tex_move = nix::Texture::load(app->directory_static << "icons/toolbar/multiview/move.png");
	tex_rotate= nix::Texture::load(app->directory_static << "icons/toolbar/multiview/rotate.png");
	tex_zoom = nix::Texture::load(app->directory_static << "icons/toolbar/multiview/zoom.png");
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

bool CameraController::Controller::hover(const vec2 &m) {
	return r.inside(m);
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
			if (c.hover(view->m))
				return true;
	}

	return r2.inside(view->m);
}

void CameraController::on_left_button_down() {
	update_rects();
	if (r_show.inside(view->m))
		show = !show;
	for (auto &c: controllers) {
		c.moving = c.r_move.inside(view->m);
		c.rotating = c.r_rotate.inside(view->m);
		c.zooming = c.r_zoom.inside(view->m);

		if (c.moving or c.rotating or c.zooming)
			MouseWrapper::start(ed->win);
	}
}

void CameraController::on_left_button_up() {
	for (auto &c: controllers) {
		if (c.moving or c.rotating or c.zooming)
			MouseWrapper::stop(ed->win);
		c.moving = c.rotating = c.zooming = false;
	}
}

void CameraController::on_mouse_move() {
	MouseWrapper::update(view);
	for (auto &c: controllers) {
		if (c.moving)
			view->cam_move_pixel(c.win, vector(view->v.x, view->v.y, 0));
		if (c.rotating)
			view->cam_rotate_pixel({view->v,0}, false);
		if (c.zooming)
			view->cam_zoom(pow(1.007f, view->v.y), false);
	}
}

void CameraController::on_mouse_wheel() {
	hui::Event *e = hui::GetEvent();

	// mouse wheel -> zoom
	view->cam_zoom(exp( - view->SPEED_ZOOM_WHEEL * e->scroll_y), view->mouse_win->type != VIEW_PERSPECTIVE);
}

void CameraController::draw_icon(const rect &rr, nix::Texture *tex, bool active) {
	nix::set_texture(tex_bg);
	if (active or rr.inside(view->m))
		set_color(ColorIconHover);
	else
		set_color(ColorIcon);
	draw_2d(rect::ID, rr, 0);
	if (tex) {
		nix::set_texture(tex);
		set_color(White);
		draw_2d(rect::ID, rr, 0);
	}
}

void CameraController::draw() {
	update_rects();
	nix::set_alpha(nix::AlphaMode::MATERIAL);

	// show/hide button
	set_color(ColorBackground);
	nix::set_texture(tex_bg);
	nix::set_shader(nix::Shader::default_2d);
	draw_2d(rect::ID, r2, 0);
	draw_icon(r_show, NULL, false);

	if (show) {

		for (auto &c: controllers) {
		// elongated background
		set_color(ColorBackground);
		nix::set_texture(tex_bg);
		draw_2d(rect(0, 0.5f, 0, 1), rect(c.r.x1, c.r.x1 + CC_RADIUS/2 + CC_BORDER, c.r.y1, c.r.y2), 0);
		draw_2d(rect(0.5f, 0.5f, 0, 1), rect(c.r.x1 + CC_RADIUS/2 + CC_BORDER, c.r.x2 - CC_RADIUS/2 - CC_BORDER, c.r.y1, c.r.y2), 0);
		draw_2d(rect(0.5f, 1, 0, 1), rect(c.r.x2 - CC_RADIUS/2 - CC_BORDER, c.r.x2, c.r.y1, c.r.y2), 0);

		// icons
		draw_icon(c.r_move, tex_move, c.moving);
		draw_icon(c.r_rotate, tex_rotate, c.rotating);
		draw_icon(c.r_zoom, tex_zoom, c.zooming);
		}
	}
	nix::set_texture(NULL);
	nix::set_alpha(nix::AlphaMode::NONE);
}

bool CameraController::in_use() {
	for (auto &c: controllers)
		if (c.moving or c.rotating or c.zooming)
			return true;
	return false;
}

};
