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
	Subscribe(data);
	Subscribe(&multi_view->selection);
}

ModeModelMeshVertex::~ModeModelMeshVertex()
{
}

void ModeModelMeshVertex::Start()
{
	OnUpdate(data);
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

void ModeModelMeshVertex::OnUpdate(Observable *o)
{
	if (this != ed->cur_mode)
		return;
	if (o->GetName() == "Data"){
		multi_view->ResetData(data);
		multi_view->SetMouseAction(0, "ActionModelMVMoveVertices", MultiView::ActionMove);
		multi_view->SetMouseAction(1, "ActionModelMVRotateVertices", MultiView::ActionRotate2d);
		multi_view->SetMouseAction(2, "ActionModelMVRotateVertices", MultiView::ActionRotate);
		/*multi_view->SetMouseAction(1, "ActionModelMVMirrorVertices", MultiView::ActionOnce);
		multi_view->SetMouseAction(2, "ActionModelMVScaleVertices", MultiView::ActionScale);*/
		multi_view->MVRectable = true;
		//CModeAll::SetMultiViewViewStage(&ViewStage, false);
		//CModeAll::SetMultiViewFunctions(&StartChanging, &EndChanging, &Change);
		multi_view->SetData(	MVDModelVertex,
				data->Vertex,
				NULL,
				MultiView::FlagDraw | MultiView::FlagIndex | MultiView::FlagSelect | MultiView::FlagMove,
				NULL, NULL);
		mode_model_mesh_skin->FillSelectionBuffers();
	}else if (o->GetName() == "MultiViewSelection"){
		// tria selection from vertices
		foreach(data->Surface, s)
			foreach(s.Triangle, t)
				t.is_selected = ((data->Vertex[t.Vertex[0]].is_selected) and (data->Vertex[t.Vertex[1]].is_selected) and (data->Vertex[t.Vertex[2]].is_selected));
		mode_model_mesh_skin->FillSelectionBuffers();
	}
}


