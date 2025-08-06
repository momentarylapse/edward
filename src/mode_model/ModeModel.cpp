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
#include <lib/xhui/controls/Toolbar.h>

yrenderer::Material* create_material(yrenderer::Context* ctx, const color& albedo, float roughness, float metal, const color& emission, bool transparent = false);

ModeModel::ModeModel(Session* session) : Mode(session) {
	multi_view = new MultiView(session);
	data = new DataModel(session);
	data->reset();
	generic_data = data.get();

	mode_mesh = new ModeMesh(this);
	mode_skeleton = new ModeSkeleton(this);
}

ModeModel::~ModeModel() = default;

void ModeModel::on_enter_rec() {
	session->out_changed >> create_sink([this] {
		update_menu();
	});

	auto win = session->win;
	auto tb = win->toolbar;
	tb->set_by_id("model-toolbar");
	win->enable("mode_model_animation", false);

	event_ids_rec.add(win->event("mode_model_mesh", [this] {
		session->set_mode(mode_mesh.get());
	}));
	event_ids_rec.add(win->event("mode_model_skeleton", [this] {
		session->set_mode(mode_skeleton.get());
	}));
	event_ids_rec.add(session->win->event("mode_properties", [this] {
		session->win->open_dialog(new ModelPropertiesDialog(session->win, data.get()));
	}));

	event_ids_rec.add(session->win->event("save", [this] {
		if (data->filename.is_empty())
			session->storage->save_as(data.get());
		else
			session->storage->save(data->filename, data.get());
	}));
	event_ids_rec.add(session->win->event("save-as", [this] {
		session->storage->save_as(data.get());
	}));
}

void ModeModel::on_leave_rec() {
	session->out_changed.unsubscribe(this);

	for (int uid: event_ids_rec)
		session->win->remove_event_handler(uid);
	event_ids_rec.clear();
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


void ModeModel::update_menu() {
	auto win = session->win;

	win->check("mode_model_mesh", session->cur_mode == mode_mesh.get());
	win->check("mode_model_skeleton", session->cur_mode == mode_skeleton.get());
}







