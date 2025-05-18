//
// Created by Michael Ankele on 2025-05-18.
//

#include "ModeExtrudePolygons.h"
#include "ModeMesh.h"
#include "../data/ModelMesh.h"
#include "../processing/MeshExtrudePolygons.h"
#include <Session.h>
#include <view/DrawingHelper.h>
#include <view/EdwardWindow.h>
#include <view/MultiView.h>



ModeExtrudePolygons::ModeExtrudePolygons(ModeMesh* parent) :
	SubMode(parent)
{
	mode_mesh = parent;
	multi_view = mode_mesh->multi_view;
	generic_data = mode_mesh->generic_data;
}

void ModeExtrudePolygons::on_enter() {
	mode_mesh->set_presentation_mode(ModeMesh::PresentationMode::Polygons);
	multi_view->set_allow_action(false);
	session->win->set_visible("overlay-button-grid-left", false);

	auto update = [this] {
		diff = mesh_prepare_extrude_polygons(*mode_mesh->data->editing_mesh, mode_mesh->data->get_selection(), dialog->get_float("distance"), dialog->is_checked("connected"));
	};

	dialog = new xhui::Panel("xxx");
	dialog->from_source(R"foodelim(
Dialog mesh-extrude-dialog "Extrude" allow-root width=200 noexpandx
	Grid ? ""
		Grid ? "" vertical class=card
			Label header "Extrude" big bold center
			Grid ? "" vertical
				Grid ? ""
					Label l-distance "Distance" right disabled
					SpinButton distance "" range=::0.1 expandx
				CheckBox connected "Keep polygons connected"
		---|
		Label ? "" expandy ignorehover
)foodelim");
	session->win->embed("overlay-main-grid", 1, 0, dialog);

	dialog->set_float("distance", 20);
	dialog->event("*", update);

	multi_view->out_selection_changed >> create_sink(update);

	update();
}

void ModeExtrudePolygons::on_leave() {
	multi_view->out_selection_changed.unsubscribe(this);
	session->win->unembed(dialog);
}


void ModeExtrudePolygons::on_draw_win(const RenderParams& params, MultiViewWindow* win) {
	mode_mesh->on_draw_win(params, win);
	auto dh = session->drawing_helper;

	dh->set_color(DrawingHelper::COLOR_X);
	dh->set_line_width(DrawingHelper::LINE_MEDIUM);
	dh->set_z_test(false);
	auto points = mesh_edit_to_lines(*mode_mesh->data->editing_mesh, diff);
	dh->draw_lines(points, false);
	dh->set_z_test(true);
}

void ModeExtrudePolygons::on_draw_post(Painter* p) {
	mode_mesh->on_draw_post(p);

	draw_info(p, "extrude...");
}


void ModeExtrudePolygons::on_key_down(int key) {
	if (key == xhui::KEY_ESCAPE) {
		session->set_mode(mode_mesh);
	}
	if (key == xhui::KEY_RETURN) {
		mode_mesh->data->edit_mesh(diff);
		session->set_mode(mode_mesh);
	}
}

void ModeExtrudePolygons::on_left_button_down(const vec2& m) {
	session->win->request_redraw();
}

void ModeExtrudePolygons::on_mouse_move(const vec2& m, const vec2& d) {
	session->win->request_redraw();
}
