//
// Created by Michael Ankele on 2025-02-07.
//

#include "ModeModel.h"
#include "mesh/ModeMesh.h"
#include "skeleton/ModeSkeleton.h"
#include "dialog/ModelPropertiesDialog.h"
#include <Session.h>
#include <storage/format/Format.h>
#include <storage/Storage.h>
#include <view/MultiView.h>
#include <view/MultiViewPanel.h>
#include <view/EdwardWindow.h>
#include <view/DocumentSession.h>

#include "lib/os/msg.h"

yrenderer::Material* create_material(yrenderer::Context* ctx, const color& albedo, float roughness, float metal, const color& emission, bool transparent = false);

ModeModel::ModeModel(DocumentSession* doc) : Mode(doc) {
	multi_view_3d = new MultiView(doc);
	multi_view_2d = new MultiView(doc);
	multi_view_2d->view_port.pos = {0.5f, 0.5f, 0};
	multi_view_2d->view_port.radius = 2;
	multi_view_panel = new MultiViewPanel(doc, multi_view_3d.get());
	multi_view = multi_view_3d.get();
	doc->set_document_panel(multi_view_panel.get());
	data = new DataModel(doc);
	data->reset();
	generic_data = data.get();
	toolbar_id = "model-toolbar";

	mode_mesh = new ModeMesh(this);
	mode_skeleton = new ModeSkeleton(this);
}

ModeModel::~ModeModel() = default;

void ModeModel::on_enter() {
	xhui::run_later(0.01f, [this] {
		doc->set_mode((Mode*)mode_mesh->mode_mesh_geometry.get());
	});
}

void ModeModel::on_enter_rec() {
	doc->out_changed >> create_sink([this] {
		on_update_menu();
	});
	data->out_changed >> multi_view->in_data_changed;

	auto win = session->win;
	win->enable("mode-animation", false);
}

void ModeModel::on_connect_events_rec() {
	doc->event("mode-mesh-geometry", [this] {
		doc->set_mode((Mode*)mode_mesh->mode_mesh_geometry.get());
	});
	doc->event("mode-mesh-sculpt", [this] {
		doc->set_mode((Mode*)mode_mesh->mode_mesh_sculpt.get());
	});
	doc->event("mode-mesh-materials", [this] {
		doc->set_mode((Mode*)mode_mesh->mode_mesh_material.get());
	});
	doc->event("mode-mesh-normals", [this] {
		doc->set_mode((Mode*)mode_mesh->mode_mesh_normals.get());
	});
	doc->event("mode-mesh-uv", [this] {
		doc->set_mode((Mode*)mode_mesh->mode_mesh_uv.get());
	});
	doc->event("mode-skeleton", [this] {
		doc->set_mode(mode_skeleton.get());
	});
	doc->event("mode_properties", [this] {
		session->win->open_dialog(new ModelPropertiesDialog(session->win, data.get()));
	});

	doc->event("save", [this] {
		if (data->filename.is_empty())
			session->storage->save_as(data.get());
		else
			session->storage->save(data->filename, data.get());
	});
	doc->event("save-as", [this] {
		session->storage->save_as(data.get());
	});
}

void ModeModel::on_leave_rec() {
	doc->out_changed.unsubscribe(this);
}

void ModeModel::on_command(const string& id) {
	if (id == "new")
		session->universal_new(FD_MODEL);
	if (id == "open")
		session->universal_open(FD_MODEL);
	if (id == "undo")
		data->undo();
	if (id == "redo")
		data->redo();
}


void ModeModel::on_update_menu() {
	auto win = session->win;

	win->check("mesh-visible0", data->editing_mesh == data->mesh.get());
	win->check("mesh-physical", data->editing_mesh == data->phys_mesh.get());

	win->check("mode-mesh-geometry", doc->cur_mode == (Mode*)mode_mesh->mode_mesh_geometry.get());
	win->check("mode-mesh-sculpt", doc->cur_mode == (Mode*)mode_mesh->mode_mesh_sculpt.get());
	win->check("mode-mesh-materials", doc->cur_mode == (Mode*)mode_mesh->mode_mesh_material.get());
	win->check("mode-mesh-normals", doc->cur_mode == (Mode*)mode_mesh->mode_mesh_normals.get());
	win->check("mode-mesh-uv", doc->cur_mode == (Mode*)mode_mesh->mode_mesh_uv.get());
	win->check("mode-skeleton", doc->cur_mode == mode_skeleton.get());

	mode_mesh->update_menu_presentation_mode();
}







