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

string file_secure(const string &filename);
string render_material(ModelMaterial *m);

ModelMaterialSelectionDialog::ModelMaterialSelectionDialog(CHuiWindow *_parent, bool _allow_parent, DataModel *_data):
	CHuiWindow("dummy", -1, -1, 320, 450, _parent, _allow_parent, HuiWinModeControls | HuiWinModeResizable, true)
{
	// dialog
	FromResource("model_material_selection_dialog");

	data = _data;
	FillMaterialList();

	SetTooltip("material_list", _("- Doppelklick um ein Material anzuwenden\n- selektieren und den Knopf \"Bearb.\" zum Bearbeiten\n- H&akchen bei \"Aktiv\" zur Auswahl f&ur folgende neue Polygone"));

	EventM("hui:close", this, (void(HuiEventHandler::*)())&ModelMaterialSelectionDialog::OnClose);
	EventMX("material_list", "hui:activate", this, (void(HuiEventHandler::*)())&ModelMaterialSelectionDialog::OnMaterialList);
	EventMX("material_list", "hui:change", this, (void(HuiEventHandler::*)())&ModelMaterialSelectionDialog::OnMaterialListCheck);
	EventMX("material_list", "hui:select", this, (void(HuiEventHandler::*)())&ModelMaterialSelectionDialog::OnMaterialListSelect);
	EventM("add_material", this, (void(HuiEventHandler::*)())&ModelMaterialSelectionDialog::OnMaterialAdd);
	EventM("edit_material", this, (void(HuiEventHandler::*)())&ModelMaterialSelectionDialog::OnMaterialEdit);

	answer = NULL;

	Subscribe(data);
}

ModelMaterialSelectionDialog::~ModelMaterialSelectionDialog()
{
	Unsubscribe(data);
	mode_model_mesh->MaterialSelectionDialog = NULL;
}

void ModelMaterialSelectionDialog::FillMaterialList()
{
	Reset("material_list");
	for (int i=0;i<data->Material.num;i++){
		int nt = 0;
		foreach(ModelSurface &s, data->Surface)
			foreach(ModelPolygon &t, s.Polygon)
			if (t.Material == i)
				nt ++;
		string im = render_material(&data->Material[i]);
		AddString("material_list", format("%d\\%d\\%s\\%s\\%s", i, nt, (i == data->CurrentMaterial) ? "true" : "false", im.c_str(), file_secure(data->Material[i].MaterialFile).c_str()));
	}
	Enable("edit_material", false);
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

void ModelMaterialSelectionDialog::OnMaterialListSelect()
{
	Enable("edit_material", GetInt("") >= 0);
}

void ModelMaterialSelectionDialog::OnMaterialAdd()
{
	data->Execute(new ActionModelAddMaterial());
}

void ModelMaterialSelectionDialog::OnMaterialEdit()
{
	int s = GetInt("material_list");
	if (s < 0)
		return;

	data->CurrentMaterial = s;
	data->CurrentTextureLevel = 0;
	mode_model->ExecuteMaterialDialog(0, false);
	FillMaterialList();
}

void ModelMaterialSelectionDialog::OnUpdate(Observable *o)
{
	FillMaterialList();
}


