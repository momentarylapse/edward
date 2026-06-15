//
// Created by Michael Ankele on 2025-03-03.
//

#include "ModeMeshSculpt.h"
#include "../ModeMesh.h"
#include "../../data/ModelMesh.h"
#include "action/ActionModelBrushExtrude.h"
#include <Session.h>
#include <lib/xhui/xhui.h>
#include <view/DocumentSession.h>
#include <view/DrawingHelper.h>
#include <view/EdwardWindow.h>
#include <view/MultiView.h>

ModeMeshSculpt::ModeMeshSculpt(ModeMesh* parent) :
	SubMode(parent)
{
	mode_mesh = parent;
	data = mode_mesh->data;
	multi_view = mode_mesh->multi_view;
	generic_data = mode_mesh->generic_data;
	toolbar_id = "model-toolbar";
	menu_id = "menu-model-sculpt";

	brush.radius = multi_view->view_port.radius * 0.07f;
	brush.depth = brush.radius * 0.3f;
}

void ModeMeshSculpt::on_enter() {
	mode_mesh->set_presentation_mode(ModeMesh::PresentationMode::Polygons);
	multi_view->set_allow_select(false);
	multi_view->set_allow_action(false);

	/*dialog = new xhui::Panel("xxx");
	dialog->from_resource("new_cube_dialog");
	set_side_panel(dialog);*/
}

void ModeMeshSculpt::on_leave() {
	//set_side_panel(nullptr);
}

void ModeMeshSculpt::on_draw_win(const yrenderer::RenderParams& params, MultiViewWindow* win) {
	mode_mesh->draw_mesh(params, win, false);
	if (target) {
		session->drawing_helper->set_color(DrawingHelper::COLOR_X);
		session->drawing_helper->set_line_width(DrawingHelper::LINE_MEDIUM);
		session->drawing_helper->draw_circle(target->pos, target->n, brush.radius);
	}
}

void ModeMeshSculpt::on_draw_post(Painter* p) {
	mode_mesh->on_draw_post(p);

	draw_info(p, "sculpt");
}


void ModeMeshSculpt::on_key_down(int key) {
}

void ModeMeshSculpt::on_mouse_move(const vec2& m, const vec2& d) {
	target = base::None;
	if (multi_view->hover and multi_view->hover->type == MultiViewType::MODEL_POLYGON) {
		const auto& p = data->mesh->polygons[multi_view->hover->index];
		target = {multi_view->hover->tp, p.temp_normal};
	}

	session->win->request_redraw();
}


void ModeMeshSculpt::on_left_button_down(const vec2& m) {
	if (!target)
		return;
	data->execute(new ActionModelBrushExtrude(target->pos, target->n, brush.radius, brush.depth));
	/*for (auto& v: data->mesh->vertices) {
		float r = (v.pos - target->pos).length();
		if (r > brush.radius)
			continue;
		float weight = exp(- (r*r) / (brush.radius*brush.radius) * 2.0f);
		v.pos += target->n * brush.depth * weight;
	}
	data->out_changed();*/

	session->win->request_redraw();
}
