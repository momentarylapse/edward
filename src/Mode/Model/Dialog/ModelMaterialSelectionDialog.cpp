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
#include "../../../Storage/Storage.h"
#include "../Mesh/ModeModelMeshTexture.h"

string file_secure(const Path &filename);
string render_material(ModelMaterial *m);

ModelMaterialSelectionDialog::ModelMaterialSelectionDialog(hui::Window *_parent, bool _allow_parent, DataModel *_data):
	hui::Dialog("model_material_selection_dialog", 400, 300, _parent, _allow_parent)
{
	from_resource("model_material_selection_dialog");
	data = _data;
	list_id = "material_list";
	fill_material_list();

	popup_materials = hui::create_resource_menu("model-material-list-popup", this);

	event("hui:close", [=]{ on_close(); });
	event("apply", [=]{ on_apply(); });
	event_x(list_id, "hui:activate", [=]{ on_apply(); });
	event_x(list_id, "hui:right-button-down", [=]{ on_material_list_right_click(); });
	event("add_new_material", [=]{ on_material_add(); });
	event("add_material", [=]{ on_material_load(); });
	event("delete_material", [=]{ on_material_delete(); });
	event("apply_material", [=]{ on_apply(); });

	answer = -1;

	data->subscribe(this, [=]{ fill_material_list(); });
}

ModelMaterialSelectionDialog::~ModelMaterialSelectionDialog() {
	data->unsubscribe(this);
	delete popup_materials;
}

void ModelMaterialSelectionDialog::fill_material_list() {
	reset(list_id);
	for (int i=0;i<data->material.num;i++) {
		int nt = 0;
		for (auto &t: data->mesh->polygon)
			if (t.material == i)
				nt ++;
		string im = render_material(data->material[i]);
		add_string(list_id, format("%d\\%d\\%s\\%s", i, nt, im, file_secure(data->material[i]->filename)));
	}
	set_int(list_id, mode_model_mesh->current_material);
}


void ModelMaterialSelectionDialog::on_close() {
	request_destroy();
}

void ModelMaterialSelectionDialog::on_apply() {
	answer = get_int(list_id);
	request_destroy();
}

void ModelMaterialSelectionDialog::on_material_list_right_click() {
	int n = hui::get_event()->row;
	if (n >= 0)
		mode_model_mesh->set_current_material(n);
	popup_materials->enable("apply_material", n >= 0);
	popup_materials->enable("delete_material", n >= 0);
	popup_materials->open_popup(this);
}

void ModelMaterialSelectionDialog::on_material_add() {
	data->execute(new ActionModelAddMaterial(""));
}

void ModelMaterialSelectionDialog::on_material_load() {
	storage->file_dialog(FD_MATERIAL, false, true, [this] {
		data->execute(new ActionModelAddMaterial(storage->dialog_file_no_ending));
	});
}

void ModelMaterialSelectionDialog::on_material_delete() {
	hui::error_box(win, "", _("not implemented yet"));
}

void ModelMaterialSelectionDialog::on_material_edit() {
	hui::error_box(win, "", _("not implemented yet"));
}



