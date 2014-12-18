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

ModelMaterialSelectionDialog::ModelMaterialSelectionDialog(HuiWindow *_parent, bool _allow_parent, DataModel *_data):
	HuiWindow("model_material_selection_dialog", _parent, _allow_parent),
	Observer("ModelMaterialSelectionDialog")
{
	data = _data;
	FillMaterialList();

	setTooltip("material_list", _("- Doppelklick um ein Material anzuwenden\n- selektieren und den Knopf \"Bearb.\" zum Bearbeiten\n- die Auswahl wird f&ur folgende neue Polygone verwendet"));

	event("hui:close", this, &ModelMaterialSelectionDialog::OnClose);
	eventX("material_list", "hui:activate", this, &ModelMaterialSelectionDialog::OnMaterialList);
	eventX("material_list", "hui:select", this, &ModelMaterialSelectionDialog::OnMaterialListSelect);
	event("add_new_material", this, &ModelMaterialSelectionDialog::OnMaterialAddNew);
	event("add_material", this, &ModelMaterialSelectionDialog::OnMaterialAdd);
	event("edit_material", this, &ModelMaterialSelectionDialog::OnMaterialEdit);

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
		foreach(ModelSurface &s, data->surface)
			foreach(ModelPolygon &t, s.polygon)
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
	delete(this);
}

void ModelMaterialSelectionDialog::OnMaterialList()
{
	if (answer)
		*answer = getInt("");
	delete(this);
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
	if (ed->fileDialog(FDMaterial, false, true))
		data->execute(new ActionModelAddMaterial(ed->DialogFileNoEnding));
}

void ModelMaterialSelectionDialog::OnMaterialEdit()
{
}

void ModelMaterialSelectionDialog::onUpdate(Observable *o, const string &message)
{
	FillMaterialList();
}


