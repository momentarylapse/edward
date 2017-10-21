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
	fromResource("model_material_selection_dialog");
	data = _data;
	FillMaterialList();

	setTooltip("material_list", _("- Doppelklick um ein Material anzuwenden\n- selektieren und den Knopf \"Bearb.\" zum Bearbeiten\n- die Auswahl wird f&ur folgende neue Polygone verwendet"));

	event("hui:close", std::bind(&ModelMaterialSelectionDialog::OnClose, this));
	eventX("material_list", "hui:activate", std::bind(&ModelMaterialSelectionDialog::OnMaterialList, this));
	eventX("material_list", "hui:select", std::bind(&ModelMaterialSelectionDialog::OnMaterialListSelect, this));
	event("add_new_material", std::bind(&ModelMaterialSelectionDialog::OnMaterialAddNew, this));
	event("add_material", std::bind(&ModelMaterialSelectionDialog::OnMaterialAdd, this));
	event("edit_material", std::bind(&ModelMaterialSelectionDialog::OnMaterialEdit, this));

	answer = NULL;

	subscribe(data);
}

ModelMaterialSelectionDialog::~ModelMaterialSelectionDialog()
{
	unsubscribe(data);
}

void ModelMaterialSelectionDialog::FillMaterialList()
{
	reset("material_list");
	for (int i=0;i<data->material.num;i++){
		int nt = 0;
		for (ModelSurface &s: data->surface)
			for (ModelPolygon &t: s.polygon)
			if (t.material == i)
				nt ++;
		string im = render_material(&data->material[i]);
		addString("material_list", format("%d\\%d\\%s\\%s", i, nt, im.c_str(), file_secure(data->material[i].material_file).c_str()));
	}
	setInt("material_list", mode_model_mesh->current_material);
}

void ModelMaterialSelectionDialog::PutAnswer(int *_answer)
{
	answer = _answer;
	if (answer)
		*answer = -1;
}


void ModelMaterialSelectionDialog::OnClose()
{
	destroy();
}

void ModelMaterialSelectionDialog::OnMaterialList()
{
	if (answer)
		*answer = getInt("");
	destroy();
}

void ModelMaterialSelectionDialog::OnMaterialListSelect()
{
	mode_model_mesh->setCurrentMaterial(getInt(""));
}

void ModelMaterialSelectionDialog::OnMaterialAddNew()
{
	data->execute(new ActionModelAddMaterial(""));
}

void ModelMaterialSelectionDialog::OnMaterialAdd()
{
	if (ed->fileDialog(FD_MATERIAL, false, true))
		data->execute(new ActionModelAddMaterial(ed->dialog_file_no_ending));
}

void ModelMaterialSelectionDialog::OnMaterialEdit()
{
}

void ModelMaterialSelectionDialog::onUpdate(Observable *o, const string &message)
{
	FillMaterialList();
}


