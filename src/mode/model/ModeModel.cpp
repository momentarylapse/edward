/*
 * ModeModel.cpp
 *
 *  Created on: 04.03.2012
 *      Author: michi
 */

#include "ModeModel.h"
#include "animation/ModeModelAnimation.h"
#include "dialog/ModelPropertiesDialog.h"
#include "mesh/ModeModelMesh.h"
#include "mesh/ModeModelMeshMaterial.h"
#include "mesh/ModeModelMeshDeform.h"
#include "mesh/ModeModelMeshPaint.h"
#include "mesh/selection/MeshSelectionModeEdge.h"
#include "mesh/selection/MeshSelectionModePolygon.h"
#include "mesh/selection/MeshSelectionModeSurface.h"
#include "mesh/selection/MeshSelectionModeVertex.h"
#include "mesh/ModeModelMeshTexture.h"
#include "skeleton/ModeModelSkeleton.h"
#include "../../EdwardWindow.h"
#include "../../Session.h"
#include "../../storage/Storage.h"
#include "../../data/model/DataModel.h"
#include "../../lib/nix/nix.h"
#include "../../multiview/MultiView.h"
#include "../../multiview/DrawingHelper.h"

ModeModel::ModeModel(Session *s, MultiView::MultiView *mv3, MultiView::MultiView *mv2) :
	Mode<ModeModel, DataModel>(s, "Model", nullptr, new DataModel(s), nullptr, "")
{
	mode_model_mesh = new ModeModelMesh(this, mv3, mv2);
	mode_model_skeleton = new ModeModelSkeleton(this, mv3);
	mode_model_animation = new ModeModelAnimation(this, mv3);
}



void ModeModel::on_start() {
	session->win->get_toolbar(hui::TOOLBAR_TOP)->set_by_id("model-toolbar");
	auto t = session->win->get_toolbar(hui::TOOLBAR_LEFT);
	t->reset();
	t->enable(false);
}



void ModeModel::on_enter() {
	session->set_mode(mode_model_mesh);
}


void ModeModel::on_end() {
	hui::Toolbar *t = session->win->get_toolbar(hui::TOOLBAR_TOP);
	t->reset();
	t->enable(false);
}



void ModeModel::on_command(const string & id) {
	if (id == "new")
		session->universal_new(FD_MODEL);
	if (id == "open")
		open();
	if (id == "save")
		save();
	if (id == "save_as")
		save_as();

	if (id == "import_from_3ds")
		import_open_3ds();
	if (id == "import_from_json")
		import_open_json();
	if (id == "import_from_ply")
		import_open_ply();

	if (id == "export_to_json")
		export_save_json();

	if (id == "undo") {
		data->undo();
		mode_model_mesh->multi_view->selection_changed_manually();
	}
	if (id == "redo") {
		data->redo();
		mode_model_mesh->multi_view->selection_changed_manually();
	}

	if (id == "mode_model_mesh")
		session->set_mode(mode_model_mesh);
	if (id == "mode_model_vertex")
		mode_model_mesh->set_selection_mode(mode_model_mesh->selection_mode_vertex);
	if (id == "mode_model_edge")
		mode_model_mesh->set_selection_mode(mode_model_mesh->selection_mode_edge);
	if (id == "mode_model_polygon")
		mode_model_mesh->set_selection_mode(mode_model_mesh->selection_mode_polygon);
	if (id == "mode_model_surface")
		mode_model_mesh->set_selection_mode(mode_model_mesh->selection_mode_surface);
	if (id == "mode_model_deform")
		session->set_mode(mode_model_mesh->mode_model_mesh_deform);
	if (id == "mode_model_paint")
		session->set_mode(mode_model_mesh->mode_model_mesh_paint);
	if (id == "mode_model_materials")
		session->set_mode(mode_model_mesh->mode_model_mesh_material);
	if (id == "mode_model_texture_coord")
		session->set_mode(mode_model_mesh->mode_model_mesh_texture);
	if (id == "mode_model_animation")
		session->set_mode(mode_model_animation);
	if (id == "mode_model_skeleton")
		session->set_mode(mode_model_skeleton);
		//SetSubMode(SubModeTextures);
	if (id == "mode_properties")
		run_properties_dialog();

	// mainly skin debugging...
/*	if (id == "detail_1")
		data->importFromTriangleSkin(1);
	if (id == "detail_2")
		data->importFromTriangleSkin(2);
	if (id == "detail_3")
		data->importFromTriangleSkin(3);*/
}



void ModeModel::on_update_menu() {
	session->win->check("mode_model_vertex", mode_model_mesh->selection_mode_vertex->is_active());
	session->win->check("mode_model_edge", mode_model_mesh->selection_mode_edge->is_active());
	session->win->check("mode_model_polygon", mode_model_mesh->selection_mode_polygon->is_active());
	session->win->check("mode_model_surface", mode_model_mesh->selection_mode_surface->is_active());
	session->win->check("mode_model_texture_coord", mode_model_mesh->mode_model_mesh_texture->is_ancestor_of(session->cur_mode));
	session->win->check("mode_model_materials", mode_model_mesh->mode_model_mesh_material->is_ancestor_of(session->cur_mode));
	session->win->check("mode_model_deform", mode_model_mesh->mode_model_mesh_deform->is_ancestor_of(session->cur_mode));
	session->win->check("mode_model_paint", mode_model_mesh->mode_model_mesh_paint->is_ancestor_of(session->cur_mode));
	session->win->check("mode_model_mesh", mode_model_mesh->is_ancestor_of(session->cur_mode)
			and !mode_model_mesh->mode_model_mesh_texture->is_ancestor_of(session->cur_mode)
			and !mode_model_mesh->mode_model_mesh_material->is_ancestor_of(session->cur_mode)
			and !mode_model_mesh->mode_model_mesh_deform->is_ancestor_of(session->cur_mode)
			and !mode_model_mesh->mode_model_mesh_paint->is_ancestor_of(session->cur_mode));
	session->win->check("mode_model_skeleton", mode_model_skeleton->is_ancestor_of(session->cur_mode));
	session->win->check("mode_model_animation", mode_model_animation->is_ancestor_of(session->cur_mode));
}



void ModeModel::_new() {
	data->reset();
	mode_model_mesh->optimize_view();
}

void ModeModel::open() {
	session->universal_open(FD_MODEL);
	/*if (!storage->open(data))
		return false;

	session->set_mode(this);
	mode_model_mesh->optimize_view();*/
}

void ModeModel::save() {
	session->storage->auto_save(data);
}

void ModeModel::save_as() {
	session->storage->save_as(data);
}

void ModeModel::import_open_3ds() {
	session->allow_termination().then([this] {
		session->storage->file_dialog(FD_FILE, false, false).then([this] (const auto& p) {
			import_load_3ds(p.complete);
		});
	});
}

bool ModeModel::import_load_3ds(const Path &filename) {
	try {
		session->storage->load(filename, data);

		session->set_mode(this);
		mode_model_mesh->optimize_view();
		return true;
	} catch(...) {
		return false;
	}
}

void ModeModel::import_open_json() {
	session->allow_termination().then([this] {
		session->storage->file_dialog(FD_FILE, false, false).then([this] (const auto& p) {
			import_load_json(p.complete);
		});
	});
}

bool ModeModel::import_load_ply(const Path &filename) {
	try {
		session->storage->load(filename, data);

		session->set_mode(this);
		mode_model_mesh->optimize_view();
		return true;
	} catch(...) {
		return false;
	}
}

void ModeModel::import_open_ply() {
	session->allow_termination().then([this] {
		session->storage->file_dialog(FD_FILE, false, false).then([this] (const auto& p) {
			import_load_ply(p.complete);
		});
	});
}

bool ModeModel::import_load_json(const Path &filename) {
	try {
		session->storage->load(filename, data);

		session->set_mode(this);
		mode_model_mesh->optimize_view();
		return true;
	} catch(...) {
		return false;
	}
}

void ModeModel::export_save_json() {
	session->storage->file_dialog(FD_FILE, true, false).then([this] (const auto& p) {
		export_write_json(p.complete);
	});
}

bool ModeModel::export_write_json(const Path &filename) {
	return session->storage->save(filename, data);
}

void ModeModel::run_properties_dialog() {
	hui::fly(new ModelPropertiesDialog(session->win, data));
}

void ModeModel::allow_selection_modes(bool allow) {
	session->win->enable("mode_model_vertex", allow);
	session->win->enable("mode_model_edge", allow);
	session->win->enable("mode_model_polygon", allow);
	session->win->enable("mode_model_surface", allow);
}
