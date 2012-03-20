/*
 * ModeModelSkeleton.cpp
 *
 *  Created on: 14.03.2012
 *      Author: michi
 */

#include "../../../Edward.h"
#include "../../../MultiView.h"
#include "ModeModelSkeleton.h"
#include "../Mesh/ModeModelMeshTriangle.h"
#include "../Animation/ModeModelAnimation.h"
#include "Creation/ModeModelSkeletonCreateBone.h"
#include "../../../Action/Model/ActionModelDeleteBoneSelection.h"


ModeModelSkeleton *mode_model_skeleton = NULL;


ModeModelSkeleton::ModeModelSkeleton(Mode *_parent, DataModel *_data)
{
	name = "ModelSkeleton";
	parent = _parent;
	data = _data;
	menu = HuiCreateResourceMenu("menu_skeleton");
	multi_view = ed->multi_view_3d;
	Subscribe(data);
	Subscribe(multi_view, "SelectionChange");
}

ModeModelSkeleton::~ModeModelSkeleton()
{
}

void ModeModelSkeleton::OnMiddleButtonUp()
{
}



void ModeModelSkeleton::OnCommand(const string & id)
{
	if (id == "skeleton_new_point")
		ed->SetCreationMode(new ModeModelSkeletonCreateBone(ed->cur_mode, data));

	if (id == "delete")
		data->Execute(new ActionModelDeleteBoneSelection(data));
}



void ModeModelSkeleton::OnRightButtonDown()
{
}



void ModeModelSkeleton::OnKeyUp()
{
}



void ModeModelSkeleton::Draw()
{
}



void ModeModelSkeleton::OnMiddleButtonDown()
{
}



void ModeModelSkeleton::OnMouseMove()
{
}



void ModeModelSkeleton::OnUpdateMenu()
{
}



void ModeModelSkeleton::Start()
{
	// relative to absolute pos
	foreach(data->Bone, b)
		if (b.Parent >= 0)
			b.pos = data->Bone[b.Parent].pos + b.DeltaPos;
		else
			b.pos = b.DeltaPos;

	OnUpdate(data);
}


void ModeModelSkeleton::OnLeftButtonDown()
{
}



void ModeModelSkeleton::End()
{
}



void ModeModelSkeleton::OnKeyDown()
{
}



void ModeModelSkeleton::OnLeftButtonUp()
{
}



void ModeModelSkeleton::OnRightButtonUp()
{
}



void ModeModelSkeleton::OnUpdate(Observable *o)
{
	if (this != ed->cur_mode)
		return;
	if (o->GetName() == "Data"){

		multi_view->ResetData(data);
		//multi_view->ResetMouseAction();

		// left -> translate
		multi_view->SetMouseAction(0, "ActionModelMVMoveBones", MultiView::ActionMove);

		//mode_model_mesh->ApplyRightMouseFunction(multi_view);
		multi_view->MVRectable = true;
		//CModeAll::SetMultiViewViewStage(&ViewStage, false);
		//CModeAll::SetMultiViewFunctions(&StartChanging, &EndChanging, &Change);
		multi_view->SetData(	MVDSkeletonPoint,
				data->Bone,
				NULL,
				MultiView::FlagDraw | MultiView::FlagIndex | MultiView::FlagSelect | MultiView::FlagMove,
				NULL, NULL);
	}else if (o->GetName() == "MultiView"){
	}
}



void DrawBone(const vector &r, const vector &d, const color &c, int win)
{
	vector pr=mode_model_skeleton->multi_view->VecProject(r,win);
	vector pd=mode_model_skeleton->multi_view->VecProject(d,win);
	if ((pr.z>0)&&(pd.z>0)&&(pr.z<1)&&(pd.z<1)){
		float z=(pr.z+pd.z)/2;
		pr.z=pd.z=0;
		vector d=pr-pd;
		//NixDrawLine(pr.x,pr.y,pd.x,pd.y,c,z);
		float l=VecLength(d);
		float w=(float)atan2(d.x,d.y)+pi;
		NixDrawLine(pr.x,pr.y,pr.x+l*(float)sin(w+0.5f)*0.2f,pr.y+l*(float)cos(w+0.5f)*0.2f,c,z);
		NixDrawLine(pr.x,pr.y,pr.x+l*(float)sin(w-0.5f)*0.2f,pr.y+l*(float)cos(w-0.5f)*0.2f,c,z);
		NixDrawLine(pd.x,pd.y,pr.x+l*(float)sin(w+0.5f)*0.2f,pr.y+l*(float)cos(w+0.5f)*0.2f,c,z);
		NixDrawLine(pd.x,pd.y,pr.x+l*(float)sin(w-0.5f)*0.2f,pr.y+l*(float)cos(w-0.5f)*0.2f,c,z);
	}
}

void DrawCoordBasis(const ModeModelSkeletonBone *b)
{
	vector o = b->pos;
	vector e[3];
	e[0] = e_x;
	e[1] = e_y;
	e[2] = e_z;
	if (ed->cur_mode == mode_model_animation)
		for (int i=0;i<3;i++)
			VecNormalTransform(e[i], b->Matrix, e[i]);
	for (int i=0;i<3;i++){
		NixDrawLine3D(o, o + e[i] * 30 / ed->multi_view_3d->zoom, color(1,0,(i==0)?1:0.5f,0));
	}
}

void ModeModelSkeleton::DrawWin(int win, irect dest)
{
	mode_model_mesh_triangle->DrawTrias();

#ifdef USE_MODELS
	// sub models
	for (int i=0;i<Bone.num;i++)
		if (Bone[i].model){
			if (SubMode==ModeModelAnimation)
				Bone[i].model->Matrix = PointMatrix[i];
			else
				MatrixTranslation(Bone[i].model->Matrix, SkeletonGetPointPos(i));
			Bone[i].model->Draw(0,false,false);
		}
#endif

	NixSetZ(false, false);
	NixEnableLighting(true);
	NixSetWire(false);

	foreach(data->Bone, b){
		/*if (b.view_stage<=ViewStage)
			continue;*/
		if (b.is_selected)
			DrawCoordBasis(&b);
		int r = b.Parent;
		if (r < 0)
			continue;
		color c = data->Bone[r].is_selected ? Red : Blue;
		if (multi_view->MouseOver == r)
			c = ColorInterpolate(c, White, 0.3f);
		DrawBone(data->Bone[r].pos, b.pos, c, win);
	}
	NixSetZ(true, true);
}


