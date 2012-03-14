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

	Reset("material_list");
	for (int i=0;i<_data->Material.num;i++){
		int nt = 0;
		foreach(_data->Surface, s)
			foreach(s.Triangle, t)
			if (t.Material == i)
				nt ++;
		string im = render_material(&_data->Material[i]);
		AddString("material_list", format("%d%s\\%d\\%s\\%s", i, (i == _data->CurrentMaterial) ? "(*)" : "", nt, im.c_str(), file_secure(_data->Material[i].MaterialFile).c_str()));
	}

	EventM("hui:close", this, (void(CHuiWindow::*)())&ModelMaterialSelectionDialog::OnClose);
	EventM("material_list", this, (void(CHuiWindow::*)())&ModelMaterialSelectionDialog::OnMaterialList);

	answer = NULL;
}

ModelMaterialSelectionDialog::~ModelMaterialSelectionDialog()
{
	mode_model_mesh->MaterialSelectionDialog = NULL;
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


