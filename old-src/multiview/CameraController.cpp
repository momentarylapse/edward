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
#include "../EdwardWindow.h"
#include "../Session.h"


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
#if HAS_LIB_GL
	tex_bg = nix::Texture::load(app->directory_static | "icons/toolbar/multiview/bg.png");
	tex_move = nix::Texture::load(app->directory_static | "icons/toolbar/multiview/move.png");
	tex_rotate= nix::Texture::load(app->directory_static | "icons/toolbar/multiview/rotate.png");
	tex_zoom = nix::Texture::load(app->directory_static | "icons/toolbar/multiview/zoom.png");
#endif
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

#if HAS_LIB_GL
	r2 = rect(nix::target_width - CC_RADIUS / 2 - CC_BORDER,
	          nix::target_width + CC_RADIUS / 2 + CC_BORDER,
	          nix::target_height / 2 - CC_RADIUS / 2 - CC_BORDER,
	          nix::target_height / 2 + CC_RADIUS / 2 + CC_BORDER);
	r_show = rect(nix::target_width - CC_RADIUS / 2,
	              nix::target_width + CC_RADIUS / 2,
	              nix::target_height / 2 - CC_RADIUS / 2,
	              nix::target_height / 2 + CC_RADIUS / 2);
#endif
}

bool CameraController::is_mouse_over() {
	update_rects();
	if (show) {
		for (auto &c: controllers) {
			if (c.hover(view->m))
				return true;
		}
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

		if (c.moving or c.rotating or c.zooming) {
			MouseWrapper::start(view->session->win);
		}
	}
}

void CameraController::on_left_button_up() {
	for (auto &c: controllers) {
		if (c.moving or c.rotating or c.zooming)
			MouseWrapper::stop(view->session->win);
		c.moving = c.rotating = c.zooming = false;
	}
}

void CameraController::on_mouse_move() {
	MouseWrapper::update(view);
	for (auto &c: controllers) {
		if (c.moving)
			view->cam_move_pixel(c.win, vec3(view->v.x, view->v.y, 0));
		if (c.rotating)
			view->cam_rotate_pixel({view->v,0}, false);
		if (c.zooming)
			view->cam_zoom(pow(1.007f, view->v.y), false);
	}
}

void CameraController::on_mouse_wheel(const vec2& scroll) {
	// mouse wheel -> zoom
	view->cam_zoom(exp( - view->SPEED_ZOOM_WHEEL * scroll.y), view->mouse_win->type != VIEW_PERSPECTIVE);
}

void CameraController::on_gesture_zoom_begin() {
	cam_radius_before_zoom = view->cam.radius;
}

void CameraController::on_gesture_zoom(float factor) {
	// mouse wheel -> zoom
//	view->cam.radius /= view->cam.radius / (cam_radius_before_zoom * factor);

	float f = view->cam.radius * factor / cam_radius_before_zoom;
	view->cam_zoom(f, view->mouse_win->type != VIEW_PERSPECTIVE);
}

void CameraController::draw_icon(const rect &rr, nix::Texture *tex, bool active) {
#if HAS_LIB_GL
	nix::bind_texture(0, tex_bg);
	if (active or rr.inside(view->m))
		view->drawing_helper->set_color(ColorIconHover);
	else
		view->drawing_helper->set_color(ColorIcon);
	view->drawing_helper->draw_2d(rect::ID, rr, 0);
	if (tex) {
		nix::bind_texture(0, tex);
		view->drawing_helper->set_color(White);
		view->drawing_helper->draw_2d(rect::ID, rr, 0);
	}
#endif
}

void CameraController::draw() {
#if HAS_LIB_GL
	update_rects();
	nix::set_alpha(nix::Alpha::SOURCE_ALPHA, nix::Alpha::SOURCE_INV_ALPHA);

	// show/hide button
	view->drawing_helper->set_color(ColorBackground);
	nix::bind_texture(0, tex_bg);
	nix::set_shader(view->ctx->default_2d.get());
	view->drawing_helper->draw_2d(rect::ID, r2, 0);
	draw_icon(r_show, nullptr, false);

	if (show) {
		for (auto &c: controllers) {
			if (c.win != view->mouse_win)
				continue;
			// elongated background
			view->drawing_helper->set_color(ColorBackground);
			nix::bind_texture(0, tex_bg);
			view->drawing_helper->draw_2d(rect(0, 0.5f, 0, 1), rect(c.r.x1, c.r.x1 + CC_RADIUS/2 + CC_BORDER, c.r.y1, c.r.y2), 0);
			view->drawing_helper->draw_2d(rect(0.5f, 0.5f, 0, 1), rect(c.r.x1 + CC_RADIUS/2 + CC_BORDER, c.r.x2 - CC_RADIUS/2 - CC_BORDER, c.r.y1, c.r.y2), 0);
			view->drawing_helper->draw_2d(rect(0.5f, 1, 0, 1), rect(c.r.x2 - CC_RADIUS/2 - CC_BORDER, c.r.x2, c.r.y1, c.r.y2), 0);

			// icons
			draw_icon(c.r_move, tex_move, c.moving);
			draw_icon(c.r_rotate, tex_rotate, c.rotating);
			draw_icon(c.r_zoom, tex_zoom, c.zooming);
		}
	}
	nix::bind_texture(0, nullptr);
	nix::disable_alpha();
#endif
}

bool CameraController::in_use() {
	for (auto &c: controllers)
		if (c.moving or c.rotating or c.zooming)
			return true;
	return false;
}

};
