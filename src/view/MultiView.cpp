//
// Created by Michael Ankele on 2025-01-20.
//

#include "MultiView.h"
#include <y/world/Camera.h>
#include <y/world/Light.h>
#include <y/y/Entity.h>
#include <y/graphics-impl.h>

MultiView::MultiView() : Renderer("multiview") {
	cam.pos = v_0;
	cam.ang = quaternion::ID;
	cam.radius = 100;
	cam.cam = new Camera();
	cam.cam->owner = new Entity;
	//cam->owner->ang = quaternion::rotation({1, 0, 0}, 0.33f);
	//cam->owner->pos = {1000,1000,-800};
	cam.cam->min_depth = 1;
	cam.cam->max_depth = 50000;
	cam.scene_view = new SceneView;
	cam.scene_view->cam = cam.cam;
}

MultiView::~MultiView() = default;


void MultiView::prepare(const RenderParams& params) {
	cam.cam->owner->ang = cam.ang;
	cam.cam->owner->pos = cam.pos - cam.cam->owner->ang * vec3::EZ * cam.radius;
	Renderer::prepare(params);
}


void MultiView::ViewPort::move(const vec3& drel) {
	pos = pos + ang * drel * radius;
}

void MultiView::ViewPort::rotate(const quaternion& qrel) {
	ang = ang * qrel;
}

