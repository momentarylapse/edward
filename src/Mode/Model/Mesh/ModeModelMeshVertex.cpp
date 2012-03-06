/*
 * ModeModelMeshVertex.cpp
 *
 *  Created on: 04.03.2012
 *      Author: michi
 */

#include "../../../Edward.h"
#include "../../../MultiView.h"
#include "ModeModelMeshVertex.h"
#include "ModeModelMeshSkin.h"
#include "../../../lib/nix/nix.h"

ModeModelMeshVertex *mode_model_mesh_vertex = NULL;

ModeModelMeshVertex::ModeModelMeshVertex(Mode *_parent, DataModel *_data)
{
	parent = _parent;
	data = _data;
	menu = HuiCreateResourceMenu("menu_model");
}

ModeModelMeshVertex::~ModeModelMeshVertex()
{
}

void ModeModelMeshVertex::Start()
{
	msg_write("model mesh vert start");

	OnDataChange();
}

void ModeModelMeshVertex::End()
{
	msg_write("model mesh vert end");
}



void ModeModelMeshVertex::DrawWin(int win, irect dest)
{
	mode_model_mesh_skin->DrawWin(win, dest);
}



void ModeModelMeshVertex::Draw()
{
	mv3d->Draw();
	NixDrawStr(100, 100, "model");
}



void ModeModelMeshVertex::OnMouseMove()
{
	mv3d->OnMouseMove();
}



void ModeModelMeshVertex::OnLeftButtonDown()
{
	mv3d->OnLeftButtonDown();
}



void ModeModelMeshVertex::OnLeftButtonUp()
{
	mv3d->OnLeftButtonUp();
}



void ModeModelMeshVertex::OnMiddleButtonDown()
{
	mv3d->OnMiddleButtonDown();
}



void ModeModelMeshVertex::OnMiddleButtonUp()
{
	mv3d->OnMiddleButtonUp();
}



void ModeModelMeshVertex::OnRightButtonDown()
{
	mv3d->OnRightButtonDown();
}



void ModeModelMeshVertex::OnRightButtonUp()
{
	mv3d->OnRightButtonUp();
}



void ModeModelMeshVertex::OnKeyDown()
{
	mv3d->OnKeyDown();
}



void ModeModelMeshVertex::OnKeyUp()
{
	mv3d->OnKeyUp();
}



void ModeModelMeshVertex::OnCommand(const string & id)
{
	mv3d->OnCommand(id);

	// TODO -> edward?
	if (id == "undo")
		data->Undo();
	if (id == "redo")
		data->Redo();
}

void ModeModelMeshVertex::OnDataChange()
{
	mv3d->ResetData();
	//CModeAll::SetMultiViewViewStage(&ViewStage, false);
	//CModeAll::SetMultiViewFunctions(&StartChanging, &EndChanging, &Change);
	mv3d->SetData(	MVDModelVertex,
					data->Vertex,
					NULL,
					mv3d->FlagDraw | mv3d->FlagIndex | mv3d->FlagSelect | mv3d->FlagMove,
					NULL, NULL);
}


