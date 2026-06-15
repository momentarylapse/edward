//
// Created by michi on 6/15/26.
//

#include "ViewPort.h"
#include "MultiView.h"
#include <lib/yrenderer/scene/SceneView.h>


ViewPort::ViewPort(MultiView* _multi_view) {
	multi_view = _multi_view;
	pos = v_0;
	ang = quaternion::rotation({1, 0, 0}, 0.33f);//quaternion::ID;
	radius = 100;
}

yrenderer::CameraParams ViewPort::cam() const {
	yrenderer::CameraParams cam;
	cam.fov = pi/4;
	cam.ang = ang;
	cam.pos = pos - ang * vec3::EZ * radius;
	cam.min_depth = radius * 0.01f;
	cam.max_depth = radius * 300;
	return cam;
}

void ViewPort::move(const vec3& drel) {
	pos = pos + ang * drel * radius;
	out_changed();
}

void ViewPort::rotate(const quaternion& qrel) {
	ang = ang * qrel;
	out_changed();
}

void ViewPort::zoom(float factor, const base::optional<vec3>& focus_point) {
	if (focus_point) {
		if (factor > 1) {
			// zoom in -> re-focus camera on plane around <focus_point>
			auto w = multi_view->hover_window;
			vec3 pos1 = w->unproject(w->project(pos).xy(), *focus_point);
			float radius1 = (pos1 - w->view_pos()).length();

			radius = radius1;
			pos = pos1;
		}

		// fixed point: <focus_point>
		radius /= factor;
		const vec3 d = *focus_point - pos;
		pos = *focus_point - d / factor;
	} else {
		radius /= factor;
	}
	out_changed();
}

void ViewPort::suggest_for_box(const Box& box) {
	pos = box.center();
	if (box.size() == v_0)
		radius = 200;
	else
		radius = box.size().length() * 1.1f;
	ang = quaternion::rotation({0.35f, 0, 0});
	out_changed();
}


