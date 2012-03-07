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
}


