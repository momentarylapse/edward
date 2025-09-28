//
// Created by Michael Ankele on 2025-05-18.
//

#include "ModeBevelEdges.h"
#include "ModeMesh.h"
#include "../data/ModelMesh.h"
#include "processing/MeshBevelEdges.h"
#include <Session.h>
#include <view/DocumentSession.h>
#include <view/DrawingHelper.h>
#include <view/EdwardWindow.h>
#include <view/MultiView.h>



ModeBevelEdges::ModeBevelEdges(ModeMesh* parent) :
	SubMode(parent)
{
	mode_mesh = parent;
	multi_view = mode_mesh->multi_view;
	generic_data = mode_mesh->generic_data;
}

void ModeBevelEdges::on_enter() {
	if (mode_mesh->presentation_mode == ModeMesh::PresentationMode::Polygons)
		mode_mesh->set_presentation_mode(ModeMesh::PresentationMode::Edges);
	multi_view->set_allow_action(false);

	auto update = [this] {
		diff = mesh_prepare_bevel_edges(*mode_mesh->data->editing_mesh, mode_mesh->multi_view->selection, dialog->get_float("radius"));
	};

	dialog = new xhui::Panel("xxx");
	dialog->from_source(R"foodelim(
Dialog mesh-bevel-dialog "Bevel" allow-root width=200 noexpandx
	Grid ? "" vertical class=card
		Label header "Bevel" big bold center
		Grid ? "" vertical
			Grid ? ""
				Label l-radius "Radius" right disabled
				SpinButton radius "" range=::0.1 expandx
		Separator ? "" horizontal
		Grid ? ""
			Label ? "" expandx
			Button cancel "Cancel" noexpandx
			Button apply "Apply" noexpandx primary
)foodelim");
	set_overlay_panel(dialog);

	dialog->set_float("radius", 5);
	dialog->event("*", update);
	dialog->event("cancel", [this] {
		request_mode_end();
	});
	dialog->event("apply", [this] {
		mode_mesh->data->edit_mesh(diff);
		request_mode_end();
	});

	multi_view->out_selection_changed >> create_sink(update);

	update();
}

void ModeBevelEdges::on_leave() {
	set_overlay_panel(nullptr);
	multi_view->out_selection_changed.unsubscribe(this);
}


void ModeBevelEdges::on_draw_win(const yrenderer::RenderParams& params, MultiViewWindow* win) {
	mode_mesh->on_draw_win(params, win);
	auto dh = session->drawing_helper;

	dh->set_color(DrawingHelper::COLOR_X);
	dh->set_line_width(DrawingHelper::LINE_MEDIUM);
	dh->set_z_test(false);
	auto points = mesh_edit_to_lines(*mode_mesh->data->editing_mesh, diff);
	dh->draw_lines(points, false);
	dh->set_z_test(true);
}

void ModeBevelEdges::on_draw_post(Painter* p) {
	mode_mesh->on_draw_post(p);

	draw_info(p, "bevel...");
}


void ModeBevelEdges::on_key_down(int key) {
	if (key == xhui::KEY_ESCAPE) {
		request_mode_end();
	}
	if (key == xhui::KEY_RETURN) {
		mode_mesh->data->edit_mesh(diff);
		request_mode_end();
	}
}

void ModeBevelEdges::on_left_button_down(const vec2& m) {
	session->win->request_redraw();
}

void ModeBevelEdges::on_mouse_move(const vec2& m, const vec2& d) {
	session->win->request_redraw();
}

