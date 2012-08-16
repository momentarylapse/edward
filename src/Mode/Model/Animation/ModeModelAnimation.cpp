/*
 * ModeModelAnimation.cpp
 *
 *  Created on: 14.03.2012
 *      Author: michi
 */

#include "../../../Edward.h"
#include "../../../MultiView.h"
#include "ModeModelAnimation.h"
#include "../Mesh/ModeModelMeshVertex.h"
#include "../Mesh/ModeModelMeshTriangle.h"
#include "../Skeleton/ModeModelSkeleton.h"
#include "../Dialog/ModelAnimationDialog.h"

ModeModelAnimation *mode_model_animation = NULL;

ModeModelAnimation::ModeModelAnimation(Mode *_parent, DataModel *_data)
{
	name = "ModelAnimation";
	parent = _parent;
	data = _data;
	menu = HuiCreateResourceMenu("menu_move");
	multi_view = ed->multi_view_3d;
	Subscribe(data);
	Subscribe(multi_view, "SelectionChange");
}


ModeModelAnimation::~ModeModelAnimation()
{
}

void ModeModelAnimation::OnDraw()
{
}



void ModeModelAnimation::OnCommand(const string & id)
{
}



void ModeModelAnimation::OnStart()
{
	multi_view->ResetData(NULL);

	// left -> translate
	//multi_view->SetMouseAction(0, "ActionModelAnimationMoveBones", MultiView::ActionMove);
	multi_view->SetMouseAction(1, "ActionModelAnimationRotateBones", MultiView::ActionRotate2d);
	multi_view->SetMouseAction(2, "ActionModelAnimationRotateBones", MultiView::ActionRotate);
	multi_view->MVRectable = true;

	// relative to absolute pos
	foreach(data->Bone, b)
		if (b.Parent >= 0)
			b.pos = data->Bone[b.Parent].pos + b.DeltaPos;
		else
			b.pos = b.DeltaPos;

	Subscribe(data);
	Subscribe(multi_view, "SelectionChange");
	OnUpdate(data);

	dialog = new ModelAnimationDialog(ed, true, data);
	dialog->Update();
}


void ModeModelAnimation::OnUpdateMenu()
{
}



void ModeModelAnimation::OnEnd()
{
	delete(dialog);
	Unsubscribe(data);
	Unsubscribe(multi_view);
	multi_view->ResetData(NULL);
}



void ModeModelAnimation::OnUpdate(Observable *o)
{
	if (o->GetName() == "Data"){

		multi_view->ResetData(data);
		//CModeAll::SetMultiViewViewStage(&ViewStage, false);

		if (data->move->Type == MoveTypeSkeletal){
			multi_view->SetData(	MVDSkeletonPoint,
					data->Bone,
					NULL,
					MultiView::FlagDraw | MultiView::FlagIndex | MultiView::FlagSelect,
					NULL, NULL);
		}else if (data->move->Type == MoveTypeVertex){
			multi_view->SetData(	MVDModelVertex,
					data->Vertex,
					NULL,
					MultiView::FlagDraw | MultiView::FlagIndex | MultiView::FlagSelect,
					NULL, NULL);
		}
	}else if (o->GetName() == "MultiView"){
	}
}



void ModeModelAnimation::OnDrawWin(int win, irect dest)
{
	if (data->move->Type == MoveTypeSkeletal){
		mode_model_skeleton->OnDrawWin(win, dest);
	}else if (data->move->Type == MoveTypeVertex){
		mode_model_mesh_vertex->OnDrawWin(win, dest);
	}else
		mode_model_mesh_triangle->DrawTrias();
}


