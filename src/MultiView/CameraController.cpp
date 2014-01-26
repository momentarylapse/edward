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
	show = false;
	tex_bg = NixLoadTexture(HuiAppDirectoryStatic + "Data/icons/toolbar/multiview/bg.png");
	tex_moving = NixLoadTexture(HuiAppDirectoryStatic + "Data/icons/toolbar/multiview/move.png");
	tex_rotating = NixLoadTexture(HuiAppDirectoryStatic + "Data/icons/toolbar/multiview/rotate.png");
}

CameraController::~CameraController()
{
}

void CameraController::UpdateRects()
{
	r = rect(NixTargetWidth - CC_MARGIN - 2 * CC_RADIUS - 3 * CC_BORDER,
	         NixTargetWidth - CC_MARGIN,
	         NixTargetHeight / 2 - CC_RADIUS / 2 - CC_BORDER,
	         NixTargetHeight / 2 + CC_RADIUS / 2 + CC_BORDER);
	r_moving = rect(NixTargetWidth - CC_MARGIN - 2 * CC_RADIUS - 2 * CC_BORDER,
	                NixTargetWidth - CC_MARGIN - CC_RADIUS - 2 * CC_BORDER,
	                NixTargetHeight / 2 - CC_RADIUS / 2,
	                NixTargetHeight / 2 + CC_RADIUS / 2);
	r_rotating = rect(NixTargetWidth - CC_MARGIN - CC_RADIUS - CC_BORDER,
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

bool CameraController::IsMouseOver()
{
	UpdateRects();
	return (show && (r.inside(impl->m.x, impl->m.y))) || r2.inside(impl->m.x, impl->m.y);
}

void CameraController::OnLeftButtonDown()
{
	UpdateRects();
	if (r_show.inside(impl->m.x, impl->m.y))
		show = !show;
	moving = r_moving.inside(impl->m.x, impl->m.y);
	rotating = r_rotating.inside(impl->m.x, impl->m.y);
}

void CameraController::OnLeftButtonUp()
{
	moving = rotating = false;
}

void CameraController::OnMouseMove()
{
	if (moving)
		impl->CamMove(impl->v);
	if (rotating)
		impl->CamRotate(impl->v, false);
}

void CameraController::Draw()
{
	UpdateRects();
	//NixEnableLighting(true);
	NixSetAlpha(AlphaMaterial);

	NixSetColor(ColorBackground);
	NixSetTexture(tex_bg);
	NixDraw2D(r_id, r2, 0);
	if (r_show.inside(impl->m.x, impl->m.y))
		NixSetColor(ColorIconHover);
	else
		NixSetColor(ColorIcon);
	NixDraw2D(r_id, r_show, 0);
	/*NixSetTexture(tex_moving);
	NixSetColor(White);
	NixDraw2D(r_id, r_moving, 0);*/

	if (show){
		NixSetColor(ColorBackground);
		NixSetTexture(tex_bg);
		NixDraw2D(r_id, r, 0);
		if (moving || r_moving.inside(impl->m.x, impl->m.y))
			NixSetColor(ColorIconHover);
		else
			NixSetColor(ColorIcon);
		NixDraw2D(r_id, r_moving, 0);
		NixSetTexture(tex_moving);
		NixSetColor(White);
		NixDraw2D(r_id, r_moving, 0);
		NixSetTexture(tex_bg);
		if (rotating || r_rotating.inside(impl->m.x, impl->m.y))
			NixSetColor(ColorIconHover);
		else
			NixSetColor(ColorIcon);
		NixDraw2D(r_id, r_rotating, 0);
		NixSetTexture(tex_rotating);
		NixSetColor(White);
		NixDraw2D(r_id, r_rotating, 0);
	}
	NixSetTexture(NULL);
	NixSetAlpha(AlphaNone);
	NixEnableLighting(false);
}

bool CameraController::InUse()
{
	return (moving || rotating);
}

};
