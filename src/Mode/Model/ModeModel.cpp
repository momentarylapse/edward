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

ModeModel *mode_model = NULL;

ModeModel::ModeModel()
{
	name = "Model";
	parent = NULL;

	menu = NULL;
	data = new DataModel;
	multi_view = NULL;

	mode_model_mesh = new ModeModelMesh(this, data);
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
		data->Reset();
	// TODO -> edward?
	if (id == "undo")
		data->Undo();
	if (id == "redo")
		data->Redo();
}



void ModeModel::OnDataChange()
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

