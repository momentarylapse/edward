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

	void Draw();
	bool InUse();

	bool show;
	bool moving, rotating;
	rect r, r_moving, r_rotating;
	rect r2, r_show;
	MultiViewImpl *impl;
	NixTexture *tex_bg;
	NixTexture *tex_moving, *tex_rotating;
};

};

#endif /* CAMERACONTROLLER_H_ */
