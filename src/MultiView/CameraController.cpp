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
const int CC_MARGIN = 100;
const int CC_BORDER = 8;

CameraController::CameraController(MultiViewImpl *_impl)
{
	impl = _impl;
	moving = false;
	rotating = false;
	show = true;
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
}

bool CameraController::IsMouseOver()
{
	UpdateRects();
	return r.inside(impl->m.x, impl->m.y);
}

void CameraController::OnLeftButtonDown()
{
	UpdateRects();
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
	NixSetColor(color(0.5f, 1, 1, 1));
	NixDraw2D(r_id, r, 0);
	if (moving || r_moving.inside(impl->m.x, impl->m.y))
		NixSetColor(color(0.7f, 0.4f, 0.4f, 1));
	else
		NixSetColor(color(0.5f, 0.2f, 0.2f, 0.8f));
	NixDraw2D(r_id, r_moving, 0);
	if (rotating || r_rotating.inside(impl->m.x, impl->m.y))
		NixSetColor(color(0.7f, 0.4f, 0.4f, 1));
	else
		NixSetColor(color(0.5f, 0.2f, 0.2f, 0.8f));
	NixDraw2D(r_id, r_rotating, 0);
	NixSetAlpha(AlphaNone);
	NixEnableLighting(false);
}

bool CameraController::InUse()
{
	return (moving || rotating);
}

};
