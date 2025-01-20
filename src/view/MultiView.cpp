//
// Created by Michael Ankele on 2025-01-20.
//

#include "MultiView.h"
#include <y/world/Camera.h>
#include <y/world/Light.h>
#include <y/y/Entity.h>
#include <y/graphics-impl.h>

MultiView::MultiView() : Renderer("multiview") {
	view_port.pos = v_0;
	view_port.ang = quaternion::ID;
	view_port.radius = 100;
	view_port.cam = new Camera();
	view_port.cam->owner = new Entity;
	//cam->owner->ang = quaternion::rotation({1, 0, 0}, 0.33f);
	//cam->owner->pos = {1000,1000,-800};
	view_port.cam->min_depth = 1;
	view_port.cam->max_depth = 50000;
	view_port.scene_view = new SceneView;
	view_port.scene_view->cam = view_port.cam;
}

MultiView::~MultiView() = default;

void MultiView::prepare(const RenderParams& params) {
	view_port.cam->owner->ang = view_port.ang;
	view_port.cam->owner->pos = view_port.pos - view_port.cam->owner->ang * vec3::EZ * view_port.radius;
	view_port.cam->update_matrices(area.width() / area.height());

	// 3d -> pixel
	projection = mat4::translation({area.x1, area.y1, 0})
		* mat4::scale(area.width()/2, area.height()/2, 1)
		* mat4::translation({1.0f, 1.0f, 0})
		* view_port.cam->m_projection * view_port.cam->m_view;

	Renderer::prepare(params);
}


void MultiView::ViewPort::move(const vec3& drel) {
	pos = pos + ang * drel * radius;
}

void MultiView::ViewPort::rotate(const quaternion& qrel) {
	ang = ang * qrel;
}

