/*
 * CameraController.h
 *
 *  Created on: 26.01.2014
 *      Author: michi
 */

#ifndef CAMERACONTROLLER_H_
#define CAMERACONTROLLER_H_


#include "../lib/math/math.h"

namespace nix{
	class Texture;
};

namespace MultiView{

class MultiView;

class CameraController
{
public:
	CameraController(MultiView *view);
	virtual ~CameraController();

	bool isMouseOver();
	void onLeftButtonDown();
	void onLeftButtonUp();
	void onMouseMove();
	void updateRects();

	void draw_icon(const rect &r, nix::Texture *tex, bool active);
	void draw();
	bool inUse();

	bool show;
	bool moving, rotating, zooming;
	rect r, r_move, r_rotate, r_zoom;
	rect r2, r_show;
	MultiView *view;
	nix::Texture *tex_bg;
	nix::Texture *tex_move, *tex_rotate, *tex_zoom;
};

};

#endif /* CAMERACONTROLLER_H_ */
