/*
 * ModelMaterialSelectionDialog.cpp
 *
 *  Created on: 14.03.2012
 *      Author: michi
 */

#include "ModelMaterialSelectionDialog.h"
#include "../../../Data/Model/DataModel.h"
#include "../Mesh/ModeModelMesh.h"
#include "../../../Action/Model/Data/ActionModelAddMaterial.h"
#include "../ModeModel.h"
#include "../../../Edward.h"
#include "../Mesh/ModeModelMeshTexture.h"

string file_secure(const string &filename);
string render_material(ModelMaterial *m);

ModelMaterialSelectionDialog::ModelMaterialSelectionDialog(hui::Window *_parent, bool _allow_parent, DataModel *_data):
	hui::Dialog("model_material_selection_dialog", 400, 300, _parent, _allow_parent),
	Observer("ModelMaterialSelectionDialog")
{
	from_resource("model_material_selection_dialog");
	data = _data;
	fill_material_list();

	popup_materials = hui::CreateResourceMenu("model-material-list-popup");

	event("hui:close", [=]{ on_close(); });
	event("apply", [=]{ on_apply(); });
	event_x("material_list", "hui:activate", [=]{ on_apply(); });
	event_x("material_list", "hui:right-button-down", [=]{ on_material_list_right_click(); });
	event("add_new_material", [=]{ on_material_add(); });
	event("add_material", [=]{ on_material_load(); });
	event("delete_material", [=]{ on_material_delete(); });
	event("apply_material", [=]{ on_apply(); });

	answer = NULL;

	subscribe(data);
}

ModelMaterialSelectionDialog::~ModelMaterialSelectionDialog() {
	unsubscribe(data);
	delete popup_materials;
}

void ModelMaterialSelectionDialog::fill_material_list()
{
	reset("material_list");
	for (int i=0;i<data->material.num;i++){
		int nt = 0;
		for (ModelSurface &s: data->surface)
			for (ModelPolygon &t: s.polygon)
			if (t.material == i)
				nt ++;
		string im = render_material(data->material[i]);
		add_string("material_list", format("%d\\%d\\%s\\%s", i, nt, im.c_str(), file_secure(data->material[i]->filename).c_str()));
	}
	set_int("material_list", mode_model_mesh->current_material);
}

void ModelMaterialSelectionDialog::put_answer(int *_answer)
{
	answer = _answer;
	if (answer)
		*answer = -1;
}


void ModelMaterialSelectionDialog::on_close() {
	destroy();
}

void ModelMaterialSelectionDialog::on_apply() {
	if (answer)
		*answer = get_int("matieral_list");
	destroy();
}

void ModelMaterialSelectionDialog::on_material_list_right_click() {
	int n = hui::GetEvent()->row;
	if (n >= 0) {
		mode_model_mesh->set_current_material(n);
	}
	popup_materials->enable("apply_material", n>=0);
	popup_materials->enable("delete_material", n>=0);
	popup_materials->open_popup(this);
}

void ModelMaterialSelectionDialog::on_material_add() {
	data->execute(new ActionModelAddMaterial(""));
}

void ModelMaterialSelectionDialog::on_material_load() {
	if (ed->file_dialog(FD_MATERIAL, false, true))
		data->execute(new ActionModelAddMaterial(ed->dialog_file_no_ending));
}

void ModelMaterialSelectionDialog::on_material_delete() {
	hui::ErrorBox(win, "", _("not implemented yet"));
}

void ModelMaterialSelectionDialog::on_material_edit() {
	hui::ErrorBox(win, "", _("not implemented yet"));
}

void ModelMaterialSelectionDialog::on_update(Observable *o, const string &message)
{
	fill_material_list();
}


