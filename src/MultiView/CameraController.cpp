/*
 * CameraController.cpp
 *
 *  Created on: 26.01.2014
 *      Author: michi
 */

#include "CameraController.h"
#include "MultiView.h"
#include "DrawingHelper.h"
#include "../lib/nix/nix.h"
#include "../Edward.h"

namespace MultiView{

const int CC_RADIUS = 50;
const int CC_MARGIN = 200;
const int CC_BORDER = 8;

const color ColorBackground = color(0.5f, 1, 1, 1);
const color ColorIcon = color(0.5f, 0.2f, 0.2f, 0.8f);
const color ColorIconHover = color(0.7f, 0.4f, 0.4f, 1);

CameraController::CameraController(MultiView *_view)
{
	view = _view;
	moving = false;
	rotating = false;
	zooming = false;
	show = false;
	tex_bg = nix::LoadTexture(app->directory_static + "icons/toolbar/multiview/bg.png");
	tex_move = nix::LoadTexture(app->directory_static + "icons/toolbar/multiview/move.png");
	tex_rotate= nix::LoadTexture(app->directory_static + "icons/toolbar/multiview/rotate.png");
	tex_zoom = nix::LoadTexture(app->directory_static + "icons/toolbar/multiview/zoom.png");
}

CameraController::~CameraController()
{
}

void CameraController::updateRects()
{
	r = rect(nix::target_width - CC_MARGIN - 3 * CC_RADIUS - 4 * CC_BORDER,
	         nix::target_width - CC_MARGIN,
	         nix::target_height / 2 - CC_RADIUS / 2 - CC_BORDER,
	         nix::target_height / 2 + CC_RADIUS / 2 + CC_BORDER);
	r_move = rect(nix::target_width - CC_MARGIN - 3 * CC_RADIUS - 3 * CC_BORDER,
	              nix::target_width - CC_MARGIN - 2 * CC_RADIUS - 3 * CC_BORDER,
	              nix::target_height / 2 - CC_RADIUS / 2,
	              nix::target_height / 2 + CC_RADIUS / 2);
	r_rotate = rect(nix::target_width - CC_MARGIN - 2 * CC_RADIUS - 2 * CC_BORDER,
	                nix::target_width - CC_MARGIN - CC_RADIUS - 2 * CC_BORDER,
	                nix::target_height / 2 - CC_RADIUS / 2,
	                nix::target_height / 2 + CC_RADIUS / 2);
	r_zoom = rect(nix::target_width - CC_MARGIN - CC_RADIUS - CC_BORDER,
	              nix::target_width - CC_MARGIN - CC_BORDER,
	              nix::target_height / 2 - CC_RADIUS / 2,
	              nix::target_height / 2 + CC_RADIUS / 2);

	r2 = rect(nix::target_width - CC_RADIUS / 2 - CC_BORDER,
	          nix::target_width + CC_RADIUS / 2 + CC_BORDER,
	          nix::target_height / 2 - CC_RADIUS / 2 - CC_BORDER,
	          nix::target_height / 2 + CC_RADIUS / 2 + CC_BORDER);
	r_show = rect(nix::target_width - CC_RADIUS / 2,
	              nix::target_width + CC_RADIUS / 2,
	              nix::target_height / 2 - CC_RADIUS / 2,
	              nix::target_height / 2 + CC_RADIUS / 2);
}

bool CameraController::isMouseOver()
{
	updateRects();
	return (show and (r.inside(view->m.x, view->m.y))) or r2.inside(view->m.x, view->m.y);
}

void CameraController::onLeftButtonDown()
{
	updateRects();
	if (r_show.inside(view->m.x, view->m.y))
		show = !show;
	moving = r_move.inside(view->m.x, view->m.y);
	rotating = r_rotate.inside(view->m.x, view->m.y);
	zooming = r_zoom.inside(view->m.x, view->m.y);
}

void CameraController::onLeftButtonUp()
{
	moving = rotating = zooming = false;
}

void CameraController::onMouseMove()
{
	if (moving)
		view->cam_move_pixel(vector(view->v.x, view->v.y, 0));
	if (rotating)
		view->cam_rotate_pixel(view->v, false);
	if (zooming)
		view->cam_zoom(pow(1.007f, view->v.y), false);
}

void CameraController::draw_icon(const rect &rr, nix::Texture *tex, bool active)
{
	nix::SetTexture(tex_bg);
	if (active or rr.inside(view->m.x, view->m.y))
		set_color(ColorIconHover);
	else
		set_color(ColorIcon);
	draw_2d(rect::ID, rr, 0);
	if (tex){
		nix::SetTexture(tex);
		set_color(White);
		draw_2d(rect::ID, rr, 0);
	}
}

void CameraController::draw() {
	updateRects();
	nix::SetAlpha(ALPHA_MATERIAL);

	// show/hide button
	set_color(ColorBackground);
	nix::SetTexture(tex_bg);
	nix::SetShader(nix::default_shader_2d);
	draw_2d(rect::ID, r2, 0);
	draw_icon(r_show, NULL, false);

	if (show) {
		// elongated background
		set_color(ColorBackground);
		nix::SetTexture(tex_bg);
		draw_2d(rect(0, 0.5f, 0, 1), rect(r.x1, r.x1 + CC_RADIUS/2 + CC_BORDER, r.y1, r.y2), 0);
		draw_2d(rect(0.5f, 0.5f, 0, 1), rect(r.x1 + CC_RADIUS/2 + CC_BORDER, r.x2 - CC_RADIUS/2 - CC_BORDER, r.y1, r.y2), 0);
		draw_2d(rect(0.5f, 1, 0, 1), rect(r.x2 - CC_RADIUS/2 - CC_BORDER, r.x2, r.y1, r.y2), 0);

		// icons
		draw_icon(r_move, tex_move, moving);
		draw_icon(r_rotate, tex_rotate, rotating);
		draw_icon(r_zoom, tex_zoom, zooming);
	}
	nix::SetTexture(NULL);
	nix::SetAlpha(ALPHA_NONE);
}

bool CameraController::inUse()
{
	return (moving or rotating or zooming);
}

};
