/*
 * ModeModelAnimationVertex.cpp
 *
 *  Created on: 16.08.2012
 *      Author: michi
 */

#include "ModeModelAnimationVertex.h"
#include "../../../Edward.h"
#include "../../../MultiView.h"
#include "../Mesh/ModeModelMeshPolygon.h"

ModeModelAnimationVertex *mode_model_animation_vertex = NULL;

ModeModelAnimationVertex::ModeModelAnimationVertex(Mode* _parent, DataModel* _data) :
	Mode("ModelAnimationVertex", _parent, _data, ed->multi_view_3d, "menu_move")
{
	data = _data;
}

ModeModelAnimationVertex::~ModeModelAnimationVertex()
{
}

void ModeModelAnimationVertex::OnStart()
{
	multi_view->ResetData(NULL);

	// left -> translate
	multi_view->SetMouseAction(0, "ActionModelAnimationMoveVertices", MultiView::ActionMove);
	multi_view->SetMouseAction(1, "ActionModelAnimationRotateVertices", MultiView::ActionRotate);
	multi_view->SetMouseAction(2, "ActionModelAnimationRotateVertices", MultiView::ActionRotate2d);
	multi_view->MVRectable = true;

	Subscribe(data);
	Subscribe(multi_view, "SelectionChange");
	OnUpdate(data);

	foreachi(ModelVertex &v, data->Vertex, i)
		vertex[i].is_selected = v.is_selected;
}

void ModeModelAnimationVertex::OnEnd()
{
	Unsubscribe(data);
	Unsubscribe(multi_view);
	multi_view->ResetData(NULL);
	vertex.clear();
}

void ModeModelAnimationVertex::OnCommand(const string& id)
{
}

void ModeModelAnimationVertex::OnUpdate(Observable* o)
{
	if (o->GetName() == "Data"){
		UpdateVertices();

		multi_view->ResetData(data);
		//CModeAll::SetMultiViewViewStage(&ViewStage, false);

		multi_view->SetData(	MVDModelVertex,
				vertex,
				NULL,
				MultiView::FlagDraw | MultiView::FlagIndex | MultiView::FlagSelect,
				NULL, NULL);
	}else if (o->GetName() == "MultiView"){
		foreachi(ModelVertex &v, data->Vertex, i)
			v.is_selected = vertex[i].is_selected;
		data->SelectionFromVertices();
	}
	mode_model_mesh_polygon->FillSelectionBuffers();
}

void ModeModelAnimationVertex::OnUpdateMenu()
{
}

void ModeModelAnimationVertex::OnDrawWin(int win)
{
	mode_model_mesh_polygon->OnDrawWin(win);
}

void ModeModelAnimationVertex::UpdateVertices()
{
	vertex.resize(data->Vertex.num);
	foreachi(ModelVertex &v, vertex, i)
		v.pos = data->Vertex[i].AnimatedPos;
}



