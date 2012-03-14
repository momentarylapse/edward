/*
 * ModeModel.cpp
 *
 *  Created on: 04.03.2012
 *      Author: michi
 */

#include "../../Edward.h"
#include "ModeModel.h"
#include "../../Data/Model/DataModel.h"
#include "Mesh/ModeModelMesh.h"
#include "Mesh/ModeModelMeshVertex.h"
#include "Mesh/ModeModelMeshSkin.h"

ModeModel *mode_model = NULL;

ModeModel::ModeModel()
{
	name = "Model";
	parent = NULL;

	menu = NULL;
	data = new DataModel;
	multi_view = NULL;
	PropertiesDialog = NULL;
	MaterialDialog = NULL;

	mode_model_mesh = new ModeModelMesh(this, data);
	Subscribe(data);
}

ModeModel::~ModeModel()
{
}

void ModeModel::OnLeftButtonDown()
{
}



void ModeModel::Start()
{
	ed->SetMode(mode_model_mesh);
}



void ModeModel::End()
{
}



void ModeModel::DrawWin(int win, irect dest)
{
}



void ModeModel::Draw()
{
}



void ModeModel::OnRightButtonDown()
{
}



void ModeModel::OnKeyDown()
{
}



void ModeModel::OnMiddleButtonDown()
{
}



void ModeModel::OnRightButtonUp()
{
}



void ModeModel::OnMouseMove()
{
}



void ModeModel::OnMiddleButtonUp()
{
}



void ModeModel::OnKeyUp()
{
}



void ModeModel::OnLeftButtonUp()
{
}



void ModeModel::OnCommand(const string & id)
{
	if (id == "new")
		New();
	if (id == "open")
		Open();
	if (id == "save")
		Save();
	if (id == "save_as")
		SaveAs();

	// TODO -> edward?
	if (id == "undo")
		data->Undo();
	if (id == "redo")
		data->Redo();

	if (id == "mode_model_vertex")
		ed->SetMode(mode_model_mesh_vertex);
	if (id == "mode_model_skin")
		ed->SetMode(mode_model_mesh_skin);
	if (id == "mode_model_texture")
		ExecuteMaterialDialog(1);
		//SetSubMode(SubModeTextures);
	if (id == "mode_model_properties")
		ExecutePropertiesDialog(0);
	if (id == "mode_model_material")
		ExecuteMaterialDialog(0);
	if (id == "mode_model_materials")
		ExecutePropertiesDialog(1);
}



void ModeModel::OnUpdate(Observable *o)
{
	data->UpdateNormals();
}



void ModeModel::SetMaterialSelected()
{
	NixSetAlpha(AlphaMaterial);
	NixSetMaterial(Black,color(0.3f,0,0,0),Black,0,Red);
}

void ModeModel::SetMaterialMouseOver()
{
	NixSetAlpha(AlphaMaterial);
	NixSetMaterial(Black,color(0.3f,0,0,0),Black,0,White);
}

void ModeModel::SetMaterialCreation()
{
	NixSetAlpha(AlphaMaterial);
	NixSetMaterial(Black, color(0.3f,0.3f,1,0.3f), Black, 0, color(1,0.1f,0.4f,0.1f));
}

void ModeModel::New()
{
	if (!ed->AllowTermination())
		return;
	data->Reset();
	ed->multi_view_2d->Reset();
	ed->multi_view_3d->Reset();
	ed->SetMode(mode_model);
}

bool ModeModel::Open()
{
	if (!ed->AllowTermination())
		return false;
	if (!ed->FileDialog(FDModel, false, false))
		return false;
	if (!data->Load(ed->DialogFileComplete))
		return false;

	ed->multi_view_2d->Reset();
	ed->multi_view_3d->Reset();
	ed->SetMode(mode_model);
	mode_model_mesh->OptimizeView();
	return true;
}

bool ModeModel::Save()
{
	if (data->filename == "")
		return SaveAs();
	return data->Save(data->filename);
}

bool ModeModel::SaveAs()
{
	if (ed->FileDialog(FDModel, true, false))
		return data->Save(ed->DialogFileComplete);
	return false;
}

void ModeModel::ExecutePropertiesDialog(int initial_tab_page)
{
	if (PropertiesDialog)
		return;

	PropertiesDialog = new ModelPropertiesDialog(ed, true, data);
	PropertiesDialog->SetInt("model_dialog_tab_control", initial_tab_page);

	PropertiesDialog->Update();
	//HuiWaitTillWindowClosed(PropertiesDialog);
}

void ModeModel::ExecuteMaterialDialog(int initial_tab_page)
{
	if (MaterialDialog)
		return;

	MaterialDialog = new ModelMaterialDialog(ed, true, data);
	MaterialDialog->SetInt("model_material_dialog_tab_control", initial_tab_page);

	MaterialDialog->Update();
	//HuiWaitTillWindowClosed(MaterialDialog);
}

