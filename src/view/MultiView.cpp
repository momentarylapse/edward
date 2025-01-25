//
// Created by Michael Ankele on 2025-01-20.
//

#include "MultiView.h"
#include "ActionController.h"
#include <Session.h>
#include <lib/xhui/Theme.h>
#include <multiview/SingleData.h>

#include "EdwardWindow.h"
#include <y/world/Camera.h>
#include <y/world/Light.h>
#include <y/y/Entity.h>
#include <y/graphics-impl.h>

MultiViewWindow::MultiViewWindow(MultiView* _multi_view) {
	multi_view = _multi_view;
}

vec3 MultiViewWindow::project(const vec3& v) const {
	return projection.project(v);
}

vec3 MultiViewWindow::dir() const {
	return multi_view->view_port.ang * vec3::EZ;
}





MultiView::MultiView(Session* s) : obs::Node<Renderer>("multiview"),
		view_port(this),
		window(this)
{
	session = s;
	resource_manager = session->resource_manager;
	active_window = &window;
	hover_window = &window;
	action_controller = new ActionController(this);

	view_port.out_changed >> create_sink([this] {
		action_controller->update();
	});
}

MultiView::~MultiView() = default;

void MultiView::set_area(const rect& _area) {
	area = _area;
	window.area = area;
}


void MultiView::prepare(const RenderParams& params) {
	view_port.cam->owner->ang = view_port.ang;
	view_port.cam->owner->pos = view_port.pos - view_port.cam->owner->ang * vec3::EZ * view_port.radius;
	view_port.cam->min_depth = view_port.radius * 0.01f;
	view_port.cam->max_depth = view_port.radius * 300;
	view_port.cam->update_matrices(area.width() / area.height());

	// 3d -> pixel
	window.projection = mat4::translation({area.x1, area.y1, 0})
		* mat4::scale(area.width()/2, area.height()/2, 1)
		* mat4::translation({1.0f, 1.0f, 0})
		* view_port.cam->m_projection * view_port.cam->m_view;

	Renderer::prepare(params);
}

void MultiView::on_mouse_move(const vec2& m, const vec2& d) {
	hover = get_hover(hover_window, m);
	//action_controller->on_mouse_move(m, d);
	// TODO if busy... return

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
	hover = base::None;
}

void MultiView::clear_selection() {
	for (auto& d: data_sets)
		for (int i=0; i<d.array->num; i++)
			reinterpret_cast<multiview::SingleData*>(d.array->simple_element(i))->is_selected = false;
	selection_box = base::None;
	out_selection_changed();
}

multiview::SingleData* MultiView::get_hover_item() {
	if (!hover)
		return nullptr;
	for (auto& d: data_sets)
		if (d.type == hover->type)
			return reinterpret_cast<multiview::SingleData*>(d.array->simple_element(hover->index));
	return nullptr;
}

void MultiView::update_selection_box() {
	// TODO per-set user override
	bool first = true;
	Box box;
	for (auto& d: data_sets)
		for (int i=0; i<d.array->num; i++) {
			auto p = reinterpret_cast<multiview::SingleData*>(d.array->simple_element(i));
			if (p->is_selected) {
				if (first) {
					box = {p->pos, p->pos};
					first = false;
				} else {
					box.min._min(p->pos);
					box.max._max(p->pos);
				}
			}
		}
	selection_box = base::None;
	if (!first)
		selection_box = box;
	action_controller->update();
}


void MultiView::on_left_button_down(const vec2& m) {
	hover = get_hover(hover_window, m);

	//action_controller->on_left_button_down(m);
	if (hover and hover->type == MultiViewType::ACTION_MANAGER) {

	} else if (auto p = get_hover_item()) {
		if (session->win->is_key_pressed(xhui::KEY_SHIFT)) {
			// toggle p
			p->is_selected = !p->is_selected;
			update_selection_box();
			out_selection_changed();
		} else if (session->win->is_key_pressed(xhui::KEY_CONTROL)) {
			// add p
			p->is_selected = true;
			update_selection_box();
			out_selection_changed();
		} else {
			// select p exclusively
			clear_selection();
			p->is_selected = true;
			update_selection_box();
			out_selection_changed();
		}
	} else {
		if (!session->win->is_key_pressed(xhui::KEY_SHIFT) and !session->win->is_key_pressed(xhui::KEY_CONTROL))
			clear_selection();
	}
}

void MultiView::on_left_button_up(const vec2& m) {
	//action_controller->on_left_button_up(m);
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
	action_controller->draw_post(p);
}


/*void MultiView::set_selection_box(const base::optional<Box>& box) {
	selection_box = box;
	out_selection_changed();
	action_controller->update();
}*/


base::optional<Hover> MultiView::get_hover(MultiViewWindow* win, const vec2& m) const {
	vec3 tp;
	auto con = action_controller->get_hover(win, m, tp);
	if (con != ActionController::Constraint::UNDEFINED)
		return Hover{MultiViewType::ACTION_MANAGER, (int)con, tp};

	if (f_hover)
		return f_hover(win, m);
	return base::None;
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


