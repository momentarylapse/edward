//
// Created by Michael Ankele on 2025-04-16.
//

#include "ModeMeshMaterial.h"
#include "../ModeMesh.h"
#include "../../data/ModelMesh.h"
#include "dialog/ModelMaterialPanel.h"
#include <Session.h>
#include <lib/os/msg.h>
#include <view/DocumentSession.h>
#include <view/EdwardWindow.h>
#include <view/MultiView.h>
#include <mode_material/dialog/MaterialSelectionDialog.h>
#include "action/ActionModelAddMaterial.h"

ModeMeshMaterial::ModeMeshMaterial(ModeMesh* parent) :
	SubMode(parent)
{
	mode_mesh = parent;
	data = mode_mesh->data;
	multi_view = mode_mesh->multi_view;
	generic_data = mode_mesh->generic_data;
	toolbar_id = "model-toolbar";
	menu_id = "menu-model-material";
}

void ModeMeshMaterial::on_enter() {
	mode_mesh->set_presentation_mode(ModeMesh::PresentationMode::Polygons);
	multi_view->set_allow_select(true);
	multi_view->set_allow_action(false);

	dialog = new ModelMaterialPanel(data);
	set_side_panel(dialog);
}

void ModeMeshMaterial::on_leave() {
	set_side_panel(nullptr);
}

void ModeMeshMaterial::on_connect_events() {
	doc->event("choose_material", [this] {
		MaterialSelectionDialog::ask(session, "Apply material", weak(data->materials), true, false).then([this] (yrenderer::Material* material) {
			int n = weak(data->materials).find(material);
			if (n >= 0) {
				// already internal
				data->apply_material(multi_view->selection, n);
				session->info("applied material to polygons");
			} else {
				data->begin_action_group("apply-material");
				data->execute(new ActionModelAddMaterial(material));
				data->apply_material(multi_view->selection, data->materials.num - 1);
				data->end_action_group();
				session->info("added material and applied to polygons");
			}
		});
	});
}

Mode* ModeMeshMaterial::get_parent() {
	return mode_mesh;
}

void ModeMeshMaterial::on_draw_win(const yrenderer::RenderParams &params, MultiViewWindow *win) {
	mode_mesh->draw_mesh(params, win, true);
}

void ModeMeshMaterial::on_draw_post(Painter* p) {
	mode_mesh->on_draw_post(p);
}


void ModeMeshMaterial::on_key_down(int key) {
}

void ModeMeshMaterial::on_command(const string& id) {
	_parent->on_command(id);
}