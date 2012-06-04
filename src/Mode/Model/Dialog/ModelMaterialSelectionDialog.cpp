/*
 * ModelMaterialSelectionDialog.cpp
 *
 *  Created on: 14.03.2012
 *      Author: michi
 */

#include "ModelMaterialSelectionDialog.h"
#include "../Mesh/ModeModelMesh.h"

string file_secure(const string &filename);
string render_material(ModeModelMaterial *m);

ModelMaterialSelectionDialog::ModelMaterialSelectionDialog(CHuiWindow *_parent, bool _allow_parent, DataModel *_data):
	CHuiWindow("dummy", -1, -1, 800, 600, _parent, _allow_parent, HuiWinModeControls | HuiWinModeResizable, true)
{
	// dialog
	FromResource("model_material_selection_dialog");

	data = _data;
	FillMaterialList();

	EventM("hui:close", this, (void(HuiEventHandler::*)())&ModelMaterialSelectionDialog::OnClose);
	EventMX("material_list", "hui:activate", this, (void(HuiEventHandler::*)())&ModelMaterialSelectionDialog::OnMaterialList);
	EventMX("material_list", "hui:change", this, (void(HuiEventHandler::*)())&ModelMaterialSelectionDialog::OnMaterialListCheck);

	answer = NULL;
}

ModelMaterialSelectionDialog::~ModelMaterialSelectionDialog()
{
	mode_model_mesh->MaterialSelectionDialog = NULL;
}

void ModelMaterialSelectionDialog::FillMaterialList()
{
	Reset("material_list");
	for (int i=0;i<data->Material.num;i++){
		int nt = 0;
		foreach(data->Surface, s)
			foreach(s.Triangle, t)
			if (t.Material == i)
				nt ++;
		string im = render_material(&data->Material[i]);
		AddString("material_list", format("%d\\%d\\%s\\%s\\%s", i, nt, (i == data->CurrentMaterial) ? "true" : "false", im.c_str(), file_secure(data->Material[i].MaterialFile).c_str()));
	}
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
		*answer = GetInt("");
	delete(this);
}

void ModelMaterialSelectionDialog::OnMaterialListCheck()
{
	data->CurrentMaterial = HuiGetEvent()->row;
	data->CurrentTextureLevel = 0;
	FillMaterialList();
}


