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
	name = "ModelMeshVertex";
	parent = _parent;
	data = _data;
	multi_view = ed->multi_view_3d;
	menu = HuiCreateResourceMenu("menu_model");
}

ModeModelMeshVertex::~ModeModelMeshVertex()
{
}

void ModeModelMeshVertex::Start()
{
	OnDataChange();
}

void ModeModelMeshVertex::End()
{
}



void ModeModelMeshVertex::DrawWin(int win, irect dest)
{
	mode_model_mesh_skin->DrawWin(win, dest);
}



void ModeModelMeshVertex::Draw()
{
	multi_view->Draw();
	NixDrawStr(100, 100, "model");
}



void ModeModelMeshVertex::OnMouseMove()
{
}



void ModeModelMeshVertex::OnLeftButtonDown()
{
}



void ModeModelMeshVertex::OnLeftButtonUp()
{
}



void ModeModelMeshVertex::OnMiddleButtonDown()
{
}



void ModeModelMeshVertex::OnMiddleButtonUp()
{
}



void ModeModelMeshVertex::OnRightButtonDown()
{
}



void ModeModelMeshVertex::OnRightButtonUp()
{
}



void ModeModelMeshVertex::OnKeyDown()
{
}



void ModeModelMeshVertex::OnKeyUp()
{
}



void ModeModelMeshVertex::OnCommand(const string & id)
{
}

void ModeModelMeshVertex::OnDataChange()
{
	multi_view->ResetData();
	multi_view->MVRectable = true;
	//CModeAll::SetMultiViewViewStage(&ViewStage, false);
	//CModeAll::SetMultiViewFunctions(&StartChanging, &EndChanging, &Change);
	multi_view->SetData(	MVDModelVertex,
			data->Vertex,
			NULL,
			MultiView::FlagDraw | MultiView::FlagIndex | MultiView::FlagSelect | MultiView::FlagMove,
			NULL, NULL);
}


