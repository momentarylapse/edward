//
// Created by Michael Ankele on 2025-01-20.
//

#include "MultiView.h"
#include "ActionController.h"
#include <Session.h>
#include <lib/xhui/Theme.h>
#include "EdwardWindow.h"
#include <y/world/Camera.h>
#include <y/world/Light.h>
#include <y/y/Entity.h>
#include <y/graphics-impl.h>

MultiView::MultiView(Session* s) : obs::Node<Renderer>("multiview"),
		view_port(this)
{
	session = s;
	resource_manager = session->resource_manager;
	action_controller = new ActionController(this);

	view_port.out_changed >> create_sink([this] {
		action_controller->update();
	});
}

MultiView::~MultiView() = default;

void MultiView::prepare(const RenderParams& params) {
	view_port.cam->owner->ang = view_port.ang;
	view_port.cam->owner->pos = view_port.pos - view_port.cam->owner->ang * vec3::EZ * view_port.radius;
	view_port.cam->min_depth = view_port.radius * 0.01f;
	view_port.cam->max_depth = view_port.radius * 300;
	view_port.cam->update_matrices(area.width() / area.height());

	// 3d -> pixel
	projection = mat4::translation({area.x1, area.y1, 0})
		* mat4::scale(area.width()/2, area.height()/2, 1)
		* mat4::translation({1.0f, 1.0f, 0})
		* view_port.cam->m_projection * view_port.cam->m_view;

	Renderer::prepare(params);
}

void MultiView::on_mouse_move(const vec2& m, const vec2& d) {
	// left -> ...
	if (session->win->button(0) and false)
		view_port.rotate(quaternion::rotation({d.y*0.003f, d.x*0.003f, 0}));

	// right -> rotate
	if (session->win->button(2))
		view_port.rotate(quaternion::rotation({d.y*0.003f, d.x*0.003f, 0}));

	// middle -> move
	if (session->win->button(1))
		view_port.move(vec3(-d.x, d.y, 0) / 800.0f); // / window size?
}

void MultiView::on_mouse_leave() {
}

void MultiView::on_mouse_wheel(const vec2& m, const vec2& d) {
	view_port.zoom(exp(d.y * 0.1f));
}

void MultiView::on_key_down(int key) {
	float d = 0.05f;
	if (key == xhui::KEY_UP)
		view_port.move({0, d, 0});
	if (key == xhui::KEY_DOWN)
		view_port.move({0, -d, 0});
	if (key == xhui::KEY_LEFT)
		view_port.move({-d, 0, 0});
	if (key == xhui::KEY_RIGHT)
		view_port.move({d, 0, 0});
}

void MultiView::on_draw(Painter* p) {
	if (selection_area) {
		p->set_color({0.2, 0,0,1});
		p->draw_rect(selection_area->canonical());
		p->set_fill(false);
		p->set_color(Blue);
		p->set_line_width(2);
		p->draw_rect(selection_area->canonical());
		p->set_fill(true);
	}

	p->set_color(Green);
	p->set_fill(false);
	p->set_line_width(4);
	p->draw_rect({200,400,200,400});
	p->set_fill(true);
	p->draw_line({300,300}, {700,700});
}


void MultiView::set_selection_box(const base::optional<Box>& box) {
	selection_box = box;
	out_selection_changed();
	action_controller->update();
}


MultiView::ViewPort::ViewPort(MultiView* _multi_view) {
	multi_view = _multi_view;
	pos = v_0;
	ang = quaternion::ID;
	radius = 100;
	cam = new Camera();
	cam->owner = new Entity;
	//cam->owner->ang = quaternion::rotation({1, 0, 0}, 0.33f);
	//cam->owner->pos = {1000,1000,-800};
	scene_view = new SceneView;
	scene_view->cam = cam;
}



void MultiView::ViewPort::move(const vec3& drel) {
	pos = pos + ang * drel * radius;
	out_changed();
}

void MultiView::ViewPort::rotate(const quaternion& qrel) {
	ang = ang * qrel;
	out_changed();
}

void MultiView::ViewPort::zoom(float factor) {
	radius /= factor;
	out_changed();
}

void MultiView::ViewPort::suggest_for_box(const vec3& vmin, const vec3& vmax) {
	pos = (vmin + vmax) / 2;
	radius = (vmax - vmin).length() * 0.7f;
	ang = quaternion::rotation({0.35f, 0, 0});
	out_changed();
}


