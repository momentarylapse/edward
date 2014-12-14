/*
 * CameraController.cpp
 *
 *  Created on: 26.01.2014
 *      Author: michi
 */

#include "CameraController.h"
#include "MultiViewImpl.h"
#include "../lib/nix/nix.h"

namespace MultiView{

const int CC_RADIUS = 50;
const int CC_MARGIN = 200;
const int CC_BORDER = 8;

const color ColorBackground = color(0.5f, 1, 1, 1);
const color ColorIcon = color(0.5f, 0.2f, 0.2f, 0.8f);
const color ColorIconHover = color(0.7f, 0.4f, 0.4f, 1);

CameraController::CameraController(MultiViewImpl *_impl)
{
	impl = _impl;
	moving = false;
	rotating = false;
	zooming = false;
	show = false;
	tex_bg = NixLoadTexture(HuiAppDirectoryStatic + "Data/icons/toolbar/multiview/bg.png");
	tex_move = NixLoadTexture(HuiAppDirectoryStatic + "Data/icons/toolbar/multiview/move.png");
	tex_rotate= NixLoadTexture(HuiAppDirectoryStatic + "Data/icons/toolbar/multiview/rotate.png");
	tex_zoom = NixLoadTexture(HuiAppDirectoryStatic + "Data/icons/toolbar/multiview/zoom.png");
}

CameraController::~CameraController()
{
}

void CameraController::updateRects()
{
	r = rect(NixTargetWidth - CC_MARGIN - 3 * CC_RADIUS - 4 * CC_BORDER,
	         NixTargetWidth - CC_MARGIN,
	         NixTargetHeight / 2 - CC_RADIUS / 2 - CC_BORDER,
	         NixTargetHeight / 2 + CC_RADIUS / 2 + CC_BORDER);
	r_move = rect(NixTargetWidth - CC_MARGIN - 3 * CC_RADIUS - 3 * CC_BORDER,
	              NixTargetWidth - CC_MARGIN - 2 * CC_RADIUS - 3 * CC_BORDER,
	              NixTargetHeight / 2 - CC_RADIUS / 2,
	              NixTargetHeight / 2 + CC_RADIUS / 2);
	r_rotate = rect(NixTargetWidth - CC_MARGIN - 2 * CC_RADIUS - 2 * CC_BORDER,
	                NixTargetWidth - CC_MARGIN - CC_RADIUS - 2 * CC_BORDER,
	                NixTargetHeight / 2 - CC_RADIUS / 2,
	                NixTargetHeight / 2 + CC_RADIUS / 2);
	r_zoom = rect(NixTargetWidth - CC_MARGIN - CC_RADIUS - CC_BORDER,
	              NixTargetWidth - CC_MARGIN - CC_BORDER,
	              NixTargetHeight / 2 - CC_RADIUS / 2,
	              NixTargetHeight / 2 + CC_RADIUS / 2);

	r2 = rect(NixTargetWidth - CC_RADIUS / 2 - CC_BORDER,
	          NixTargetWidth + CC_RADIUS / 2 + CC_BORDER,
	          NixTargetHeight / 2 - CC_RADIUS / 2 - CC_BORDER,
	          NixTargetHeight / 2 + CC_RADIUS / 2 + CC_BORDER);
	r_show = rect(NixTargetWidth - CC_RADIUS / 2,
	              NixTargetWidth + CC_RADIUS / 2,
	              NixTargetHeight / 2 - CC_RADIUS / 2,
	              NixTargetHeight / 2 + CC_RADIUS / 2);
}

bool CameraController::isMouseOver()
{
	updateRects();
	return (show && (r.inside(impl->m.x, impl->m.y))) || r2.inside(impl->m.x, impl->m.y);
}

void CameraController::onLeftButtonDown()
{
	updateRects();
	if (r_show.inside(impl->m.x, impl->m.y))
		show = !show;
	moving = r_move.inside(impl->m.x, impl->m.y);
	rotating = r_rotate.inside(impl->m.x, impl->m.y);
	zooming = r_zoom.inside(impl->m.x, impl->m.y);
}

void CameraController::onLeftButtonUp()
{
	moving = rotating = zooming = false;
}

void CameraController::onMouseMove()
{
	if (moving)
		impl->CamMove(impl->v);
	if (rotating)
		impl->CamRotate(impl->v, false);
	if (zooming)
		impl->CamZoom(pow(1.007f, impl->v.y), false);
}

void CameraController::draw_icon(const rect &rr, NixTexture *tex, bool active)
{
	NixSetTexture(tex_bg);
	if (active || rr.inside(impl->m.x, impl->m.y))
		NixSetColor(ColorIconHover);
	else
		NixSetColor(ColorIcon);
	NixDraw2D(r_id, rr, 0);
	if (tex){
		NixSetTexture(tex);
		NixSetColor(White);
		NixDraw2D(r_id, rr, 0);
	}
}

void CameraController::draw()
{
	updateRects();
	//NixEnableLighting(true);
	NixSetAlpha(AlphaMaterial);

	// show/hide button
	NixSetColor(ColorBackground);
	NixSetTexture(tex_bg);
	NixDraw2D(r_id, r2, 0);
	draw_icon(r_show, NULL, false);

	if (show){
		// elongated background
		NixSetColor(ColorBackground);
		NixSetTexture(tex_bg);
		NixDraw2D(rect(0, 0.5f, 0, 1), rect(r.x1, r.x1 + CC_RADIUS/2 + CC_BORDER, r.y1, r.y2), 0);
		NixDraw2D(rect(0.5f, 0.5f, 0, 1), rect(r.x1 + CC_RADIUS/2 + CC_BORDER, r.x2 - CC_RADIUS/2 - CC_BORDER, r.y1, r.y2), 0);
		NixDraw2D(rect(0.5f, 1, 0, 1), rect(r.x2 - CC_RADIUS/2 - CC_BORDER, r.x2, r.y1, r.y2), 0);

		// icons
		draw_icon(r_move, tex_move, moving);
		draw_icon(r_rotate, tex_rotate, rotating);
		draw_icon(r_zoom, tex_zoom, zooming);
	}
	NixSetTexture(NULL);
	NixSetAlpha(AlphaNone);
	NixEnableLighting(false);
}

bool CameraController::inUse()
{
	return (moving || rotating || zooming);
}

};
