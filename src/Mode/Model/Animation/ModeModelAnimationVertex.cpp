/*
 * ModeModelAnimationVertex.cpp
 *
 *  Created on: 16.08.2012
 *      Author: michi
 */

#include "ModeModelAnimationVertex.h"
#include "../../../Edward.h"
#include "../../../MultiView.h"
#include "../Mesh/ModeModelMeshVertex.h"
#include "../Mesh/ModeModelMeshTriangle.h"

ModeModelAnimationVertex *mode_model_animation_vertex = NULL;

ModeModelAnimationVertex::ModeModelAnimationVertex(Mode* _parent, DataModel* _data)
{
	name = "ModelAnimationVertex";
	parent = _parent;
	data = _data;
	menu = HuiCreateResourceMenu("menu_move");
	multi_view = ed->multi_view_3d;
	Subscribe(data);
	Subscribe(multi_view, "SelectionChange");
}

ModeModelAnimationVertex::~ModeModelAnimationVertex()
{
}

void ModeModelAnimationVertex::OnStart()
{
	multi_view->ResetData(NULL);

	// left -> translate
	//multi_view->SetMouseAction(0, "ActionModelAnimationMoveVertices", MultiView::ActionMove);
//	multi_view->SetMouseAction(1, "ActionModelAnimationRotateVertices", MultiView::ActionRotate2d);
//	multi_view->SetMouseAction(2, "ActionModelAnimationRotateVertices", MultiView::ActionRotate);
	multi_view->MVRectable = true;

	Subscribe(data);
	Subscribe(multi_view, "SelectionChange");
	OnUpdate(data);
}

void ModeModelAnimationVertex::OnEnd()
{
	Unsubscribe(data);
	Unsubscribe(multi_view);
	multi_view->ResetData(NULL);
}

void ModeModelAnimationVertex::OnCommand(const string& id)
{
}

void ModeModelAnimationVertex::OnUpdate(Observable* o)
{
	if (o->GetName() == "Data"){

		multi_view->ResetData(data);
		//CModeAll::SetMultiViewViewStage(&ViewStage, false);

		multi_view->SetData(	MVDModelVertex,
				data->Vertex,
				NULL,
				MultiView::FlagDraw | MultiView::FlagIndex | MultiView::FlagSelect,
				NULL, NULL);
	}else if (o->GetName() == "MultiView"){
	}
}

void ModeModelAnimationVertex::OnUpdateMenu()
{
}

void ModeModelAnimationVertex::OnDrawWin(int win, irect dest)
{
	mode_model_mesh_vertex->OnDrawWin(win, dest);
}


