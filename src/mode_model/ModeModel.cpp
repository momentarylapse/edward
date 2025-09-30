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
#include <view/EdwardWindow.h>
#include <view/DocumentSession.h>
#include <lib/xhui/controls/Toolbar.h>

#include "lib/os/msg.h"

yrenderer::Material* create_material(yrenderer::Context* ctx, const color& albedo, float roughness, float metal, const color& emission, bool transparent = false);

ModeModel::ModeModel(DocumentSession* doc) : Mode(doc) {
	auto mvp = new MultiViewPanel(doc);
	multi_view = mvp->multi_view;
	doc->set_document_panel(mvp);
	data = new DataModel(doc);
	data->reset();
	generic_data = data.get();

	mode_mesh = new ModeMesh(this);
	mode_skeleton = new ModeSkeleton(this);
}

ModeModel::~ModeModel() = default;

void ModeModel::on_set_menu() {
	auto win = session->win;
	auto tb = win->tool_bar;
	tb->set_by_id("model-toolbar");
}


void ModeModel::on_enter_rec() {
	doc->out_changed >> create_sink([this] {
		on_update_menu();
	});

	auto win = session->win;
	win->enable("mode_model_animation", false);
}

void ModeModel::on_connect_events_rec() {
	doc->event("mode_model_mesh", [this] {
		doc->set_mode(mode_mesh.get());
	});
	doc->event("mode_model_skeleton", [this] {
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

	win->check("mode_model_mesh", doc->cur_mode == mode_mesh.get());
	win->check("mesh-visible0", data->editing_mesh == data->mesh.get());
	win->check("mesh-physical", data->editing_mesh == data->phys_mesh.get());

	win->check("mode_model_deform", doc->cur_mode == (Mode*)mode_mesh->mode_mesh_sculpt.get());
	win->check("mode_model_materials", doc->cur_mode == (Mode*)mode_mesh->mode_mesh_material.get());
	win->check("mode_model_skeleton", doc->cur_mode == mode_skeleton.get());

	mode_mesh->update_menu_presentation_mode();
}







