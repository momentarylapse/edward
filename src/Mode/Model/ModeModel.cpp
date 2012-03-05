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
#include "../../Action/Model/ActionAddVertex.h"

ModeModel *mode_model = NULL;

ModeModel::ModeModel()
{
	parent = NULL;

	menu = NULL;
	data = new DataModel;

	mode_model_mesh = new ModeModelMesh(this, data);

	data->Execute(new ActionAddVertex(vector(0, 0, 0), -1));
	data->Execute(new ActionAddVertex(vector(20, 0, 0), -1));
	data->Execute(new ActionAddVertex(vector(20, 20, 0), -1));
	data->Execute(new ActionAddVertex(vector(0, 20, 0), -1));
	data->Execute(new ActionAddVertex(vector(0, 0, 10), -1));
}

ModeModel::~ModeModel()
{
}

void ModeModel::OnLeftButtonDown()
{
}



void ModeModel::Start()
{
	msg_write("model start");
	ed->SetMode(mode_model_mesh);
}



void ModeModel::End()
{
	msg_write("model end");
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
}



void ModeModel::OnDataChange()
{
}


