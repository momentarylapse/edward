/*
 * ModeModelAnimationVertex.cpp
 *
 *  Created on: 16.08.2012
 *      Author: michi
 */

#include "ModeModelAnimationVertex.h"
#include "ModeModelAnimation.h"
#include "../../../Edward.h"
#include "../../../MultiView/MultiView.h"
#include "../Mesh/ModeModelMeshPolygon.h"

ModeModelAnimationVertex *mode_model_animation_vertex = NULL;

ModeModelAnimationVertex::ModeModelAnimationVertex(ModeBase* _parent) :
	Mode<DataModel>("ModelAnimationVertex", _parent, ed->multi_view_3d, "menu_move")
{
}

ModeModelAnimationVertex::~ModeModelAnimationVertex()
{
}

void ModeModelAnimationVertex::OnStart()
{
	multi_view->ClearData(NULL);

	// left -> translate
	multi_view->SetMouseAction("ActionModelAnimationMoveVertices", MultiView::ActionMove);
//	multi_view->SetMouseAction("ActionModelAnimationRotateVertices", MultiView::ActionRotate);
//	multi_view->SetMouseAction("ActionModelAnimationRotateVertices", MultiView::ActionRotate2d);
	multi_view->allow_rect = true;

	Subscribe(data);
	Subscribe(multi_view, "SelectionChange");
	OnUpdate(data);
}

void ModeModelAnimationVertex::OnEnd()
{
	Unsubscribe(data);
	Unsubscribe(multi_view);
	multi_view->ClearData(NULL);
}

void ModeModelAnimationVertex::OnCommand(const string& id)
{
}

void ModeModelAnimationVertex::OnUpdate(Observable* o)
{
	if (o->GetName() == "Data"){
		UpdateVertices();

		multi_view->ClearData(data);
		//CModeAll::SetMultiViewViewStage(&ViewStage, false);

		multi_view->AddData(	MVDModelVertex,
				mode_model_animation->vertex,
				NULL,
				MultiView::FlagDraw | MultiView::FlagIndex | MultiView::FlagSelect);
	}else if (o->GetName() == "MultiView"){
		foreachi(ModelVertex &v, data->Vertex, i)
			v.is_selected = mode_model_animation->vertex[i].is_selected;
		data->SelectionFromVertices();
	}
	mode_model_mesh_polygon->FillSelectionBuffers(mode_model_animation->vertex);
}

void ModeModelAnimationVertex::OnUpdateMenu()
{
}

void ModeModelAnimationVertex::OnDrawWin(MultiView::Window *win)
{
	msg_db_f("skin.DrawWin",4);

	mode_model_mesh_polygon->DrawPolygons(win, mode_model_animation->vertex);
	NixSetShader(NULL);
	NixSetWire(false);
	NixSetZ(true,true);
	NixSetAlpha(AlphaNone);
	NixEnableLighting(true);
	msg_db_m("----a",4);

	mode_model_mesh_polygon->SetMaterialMarked();
	NixDraw3D(mode_model_mesh_polygon->VBMarked);
	NixSetMaterial(White,White,Black,0,Black);
	NixSetAlpha(AlphaNone);
}

void ModeModelAnimationVertex::UpdateVertices()
{
	// deprecated by mode_model_animation->vertex
	/*vertex.resize(data->Vertex.num);
	foreachi(ModelVertex &v, vertex, i)
		v.pos = data->Vertex[i].AnimatedPos;*/
}



