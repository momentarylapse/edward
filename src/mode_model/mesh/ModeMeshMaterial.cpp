//
// Created by Michael Ankele on 2025-04-16.
//

#include "ModeMeshMaterial.h"
#include "ModeMesh.h"
#include "../data/ModelMesh.h"
#include "../dialog/ModelMaterialPanel.h"
#include "../action/mesh/sculpt/ActionModelBrushExtrude.h"
#include <Session.h>
#include <data/mesh/GeometryCube.h>
#include <lib/os/msg.h>
#include <lib/xhui/config.h>
#include <lib/xhui/Theme.h>
#include <lib/xhui/xhui.h>
#include <view/DrawingHelper.h>
#include <view/EdwardWindow.h>
#include <view/MultiView.h>

ModeMeshMaterial::ModeMeshMaterial(ModeMesh* parent) :
	Mode(parent->session)
{
	mode_mesh = parent;
	data = mode_mesh->data;
	multi_view = mode_mesh->multi_view;
	generic_data = mode_mesh->generic_data;
}

void ModeMeshMaterial::on_enter() {
	mode_mesh->set_presentation_mode(ModeMesh::PresentationMode::Polygons);
	multi_view->set_allow_select(true);
	//multi_view->set_allow_action(false);

	session->win->set_visible("overlay-button-grid-left", false);

	dialog = new ModelMaterialPanel(data);
	set_side_panel(dialog);

	data->out_changed >> create_sink([this] {
		mode_mesh->update_vb();
		session->win->request_redraw();
	});
}

void ModeMeshMaterial::on_leave() {
	//session->win->unembed(dialog);
	set_side_panel(nullptr);

	data->out_changed.unsubscribe(this);
}


Mode* ModeMeshMaterial::get_parent() {
	return mode_mesh;
}


void ModeMeshMaterial::on_prepare_scene(const RenderParams& params) {
	mode_mesh->on_prepare_scene(params);
}

void ModeMeshMaterial::on_draw_win(const RenderParams& params, MultiViewWindow* win) {
	mode_mesh->on_draw_win(params, win);
}

void ModeMeshMaterial::on_key_down(int key) {
	if (key == xhui::KEY_ESCAPE) {
		session->set_mode(mode_mesh);
	}
}

void ModeMeshMaterial::on_command(const string& id) {
/*	if (id == "new")
		session->universal_new(FD_MODEL);
	if (id == "open")
		session->universal_open(FD_MODEL);*/
	if (id == "undo")
		data->undo();
	if (id == "redo")
		data->redo();
}