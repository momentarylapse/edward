/*
 * CameraController.h
 *
 *  Created on: 26.01.2014
 *      Author: michi
 */

#ifndef CAMERACONTROLLER_H_
#define CAMERACONTROLLER_H_


#include "../lib/math/math.h"

class NixTexture;

namespace MultiView{

class MultiViewImpl;

class CameraController
{
public:
	CameraController(MultiViewImpl *impl);
	virtual ~CameraController();

	bool IsMouseOver();
	void OnLeftButtonDown();
	void OnLeftButtonUp();
	void OnMouseMove();
	void UpdateRects();

	void draw_icon(const rect &r, NixTexture *tex, bool active);
	void Draw();
	bool InUse();

	bool show;
	bool moving, rotating, zooming;
	rect r, r_move, r_rotate, r_zoom;
	rect r2, r_show;
	MultiViewImpl *impl;
	NixTexture *tex_bg;
	NixTexture *tex_move, *tex_rotate, *tex_zoom;
};

};

#endif /* CAMERACONTROLLER_H_ */
