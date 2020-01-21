/*
 * ModeModel.cpp
 *
 *  Created on: 04.03.2012
 *      Author: michi
 */

#include "../../Edward.h"
#include "../../Storage/Storage.h"
#include "ModeModel.h"
#include "../../Data/Model/DataModel.h"
#include "Mesh/ModeModelMesh.h"
#include "Mesh/ModeModelMeshMaterial.h"
#include "Mesh/ModeModelMeshDeform.h"
#include "Mesh/ModeModelMeshPaint.h"
#include "Skeleton/ModeModelSkeleton.h"
#include "Animation/ModeModelAnimation.h"
#include "../../lib/nix/nix.h"
#include "../../MultiView/MultiView.h"
#include "Mesh/Selection/MeshSelectionModeEdge.h"
#include "Mesh/Selection/MeshSelectionModePolygon.h"
#include "Mesh/Selection/MeshSelectionModeSurface.h"
#include "Mesh/Selection/MeshSelectionModeVertex.h"
#include "Mesh/ModeModelMeshTexture.h"

ModeModel *mode_model = NULL;

ModeModel::ModeModel() :
	Mode<DataModel>("Model", NULL, new DataModel, NULL, "")
{
	properties_dialog = NULL;

	mode_model_mesh = new ModeModelMesh(this);
	mode_model_skeleton = new ModeModelSkeleton(this);
	mode_model_animation = new ModeModelAnimation(this);
}

ModeModel::~ModeModel()
{
}



void ModeModel::on_start()
{
	ed->toolbar[hui::TOOLBAR_TOP]->set_by_id("model-toolbar");
	auto t = ed->toolbar[hui::TOOLBAR_LEFT];
	t->reset();
	t->enable(false);
}



void ModeModel::on_enter()
{
	ed->set_mode(mode_model_mesh);
}


void ModeModel::on_end()
{
	if (properties_dialog)
		delete(properties_dialog);
	properties_dialog = NULL;

	hui::Toolbar *t = ed->toolbar[hui::TOOLBAR_TOP];
	t->reset();
	t->enable(false);
}



void ModeModel::on_command(const string & id)
{
	if (id == "new")
		_new();
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

	// TODO -> edward?
	if (id == "undo")
		data->undo();
	if (id == "redo")
		data->redo();

	if (id == "mode_model_mesh")
		ed->set_mode(mode_model_mesh);
	if (id == "mode_model_vertex")
		mode_model_mesh->set_selection_mode(mode_model_mesh->selection_mode_vertex);
	if (id == "mode_model_edge")
		mode_model_mesh->set_selection_mode(mode_model_mesh->selection_mode_edge);
	if (id == "mode_model_polygon")
		mode_model_mesh->set_selection_mode(mode_model_mesh->selection_mode_polygon);
	if (id == "mode_model_surface")
		mode_model_mesh->set_selection_mode(mode_model_mesh->selection_mode_surface);
	if (id == "mode_model_deform")
		ed->set_mode(mode_model_mesh_deform);
	if (id == "mode_model_paint")
		ed->set_mode(mode_model_mesh_paint);
	if (id == "mode_model_materials")
		ed->set_mode(mode_model_mesh_material);
	if (id == "mode_model_texture_coord")
		ed->set_mode(mode_model_mesh_texture);
	if (id == "mode_model_animation")
		ed->set_mode(mode_model_animation);
	if (id == "mode_model_skeleton")
		ed->set_mode(mode_model_skeleton);
		//SetSubMode(SubModeTextures);
	if (id == "mode_properties")
		execute_properties_dialog();

	// mainly skin debugging...
/*	if (id == "detail_1")
		data->importFromTriangleSkin(1);
	if (id == "detail_2")
		data->importFromTriangleSkin(2);
	if (id == "detail_3")
		data->importFromTriangleSkin(3);*/
}



void ModeModel::on_update_menu()
{
	ed->check("mode_model_vertex", mode_model_mesh->selection_mode_vertex->is_active());
	ed->check("mode_model_edge", mode_model_mesh->selection_mode_edge->is_active());
	ed->check("mode_model_polygon", mode_model_mesh->selection_mode_polygon->is_active());
	ed->check("mode_model_surface", mode_model_mesh->selection_mode_surface->is_active());
	ed->check("mode_model_texture_coord", mode_model_mesh_texture->is_ancestor_of(ed->cur_mode));
	ed->check("mode_model_materials", mode_model_mesh_material->is_ancestor_of(ed->cur_mode));
	ed->check("mode_model_deform", mode_model_mesh_deform->is_ancestor_of(ed->cur_mode));
	ed->check("mode_model_paint", mode_model_mesh_paint->is_ancestor_of(ed->cur_mode));
	ed->check("mode_model_mesh", mode_model_mesh->is_ancestor_of(ed->cur_mode) and !mode_model_mesh_texture->is_ancestor_of(ed->cur_mode) and !mode_model_mesh_material->is_ancestor_of(ed->cur_mode) and !mode_model_mesh_deform->is_ancestor_of(ed->cur_mode) and !mode_model_mesh_paint->is_ancestor_of(ed->cur_mode));
	ed->check("mode_model_skeleton", mode_model_skeleton->is_ancestor_of(ed->cur_mode));
	ed->check("mode_model_animation", mode_model_animation->is_ancestor_of(ed->cur_mode));
}



void ModeModel::set_material_selected()
{
	nix::SetAlpha(ALPHA_MATERIAL);
	nix::SetShader(MultiView::shader_selection);
	nix::SetMaterial(Black,color(0.3f,0,0,0),Black,0,Red);
	//nix::SetMaterial(Black,color(0.3f,0,0,0),Black,0,color(1, 0.5f, 0, 1));
	nix::SetTexture(NULL);
}

void ModeModel::set_material_hover()
{
	nix::SetAlpha(ALPHA_MATERIAL);
	nix::SetShader(MultiView::shader_selection);
	nix::SetMaterial(Black,color(0.5f,0,0,0),Black,0,White);
	nix::SetTexture(NULL);
}

void ModeModel::set_material_creation(float intensity)
{
	nix::SetAlpha(ALPHA_MATERIAL);
	nix::SetShader(MultiView::shader_selection);
	nix::SetMaterial(Black, color(0.3f*intensity,0.3f,1,0.3f), Black, 0, color(1,0.1f,0.4f,0.1f));
	nix::SetTexture(NULL);
}

void ModeModel::_new()
{
	if (!ed->allow_termination())
		return;
	data->reset();
	ed->set_mode(this);
	mode_model_mesh->optimize_view();
}

bool ModeModel::open() {
	if (!storage->open(data))
		return false;

	ed->set_mode(this);
	mode_model_mesh->optimize_view();
	return true;
}

bool ModeModel::save() {
	return storage->auto_save(data);
}

bool ModeModel::save_as() {
	return storage->save_as(data);
}

bool ModeModel::import_open_3ds()
{
	if (!ed->allow_termination())
		return false;
	if (!storage->file_dialog(FD_FILE, false, false))
		return false;
	return import_load_3ds(storage->dialog_file_complete);
}

bool ModeModel::import_load_3ds(const string &filename)
{
	try {
		storage->load(filename, data);

		ed->set_mode(this);
		mode_model_mesh->optimize_view();
		return true;
	} catch(...) {
		return false;
	}
}

bool ModeModel::import_open_json()
{
	if (!ed->allow_termination())
		return false;
	if (!storage->file_dialog(FD_FILE, false, false))
		return false;
	return import_load_json(storage->dialog_file_complete);
}

bool ModeModel::import_load_ply(const string &filename)
{
	try {
		storage->load(filename, data);

		ed->set_mode(this);
		mode_model_mesh->optimize_view();
		return true;
	} catch(...) {
		return false;
	}
}

bool ModeModel::import_open_ply()
{
	if (!ed->allow_termination())
		return false;
	if (!storage->file_dialog(FD_FILE, false, false))
		return false;
	return import_load_ply(storage->dialog_file_complete);
}

bool ModeModel::import_load_json(const string &filename) {
	try {
		storage->load(filename, data);

		ed->set_mode(this);
		mode_model_mesh->optimize_view();
		return true;
	} catch(...) {
		return false;
	}
}

bool ModeModel::export_save_json()
{
	if (!storage->file_dialog(FD_FILE, true, false))
		return false;
	return export_write_json(storage->dialog_file_complete);
}

bool ModeModel::export_write_json(const string &filename)
{
	storage->save(filename, data);
	return true;
}

void ModeModel::execute_properties_dialog()
{
	if (properties_dialog){
		if (!properties_dialog->active){
			properties_dialog->restart();
			properties_dialog->show();
		}
		return;
	}

	properties_dialog = new ModelPropertiesDialog(ed, true, data);

	properties_dialog->show();
}

void ModeModel::allow_selection_modes(bool allow)
{
	ed->enable("mode_model_vertex", allow);
	ed->enable("mode_model_edge", allow);
	ed->enable("mode_model_polygon", allow);
	ed->enable("mode_model_surface", allow);
}
