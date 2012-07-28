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
#include "../../../Action/Model/Skeleton/ActionModelDeleteBoneSelection.h"


ModeModelSkeleton *mode_model_skeleton = NULL;


ModeModelSkeleton::ModeModelSkeleton(Mode *_parent, DataModel *_data)
{
	name = "ModelSkeleton";
	parent = _parent;
	data = _data;
	menu = HuiCreateResourceMenu("menu_skeleton");
	multi_view = ed->multi_view_3d;
}

ModeModelSkeleton::~ModeModelSkeleton()
{
}



void ModeModelSkeleton::OnCommand(const string & id)
{
	if (id == "skeleton_new_point")
		ed->SetMode(new ModeModelSkeletonCreateBone(ed->cur_mode));

	if (id == "delete")
		data->Execute(new ActionModelDeleteBoneSelection(data));
}



void ModeModelSkeleton::OnDraw()
{
}



void ModeModelSkeleton::OnUpdateMenu()
{
}



void ModeModelSkeleton::OnStart()
{
	// relative to absolute pos
	foreach(data->Bone, b)
		if (b.Parent >= 0)
			b.pos = data->Bone[b.Parent].pos + b.DeltaPos;
		else
			b.pos = b.DeltaPos;

	Subscribe(data);
	Subscribe(multi_view, "SelectionChange");
	OnUpdate(data);
}



void ModeModelSkeleton::OnEnd()
{
	Unsubscribe(data);
	Unsubscribe(multi_view);
}



void ModeModelSkeleton::OnUpdate(Observable *o)
{
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
		NixSetColor(c);
		NixDrawLine(pr.x,pr.y,pr.x+l*(float)sin(w+0.5f)*0.2f,pr.y+l*(float)cos(w+0.5f)*0.2f,z);
		NixDrawLine(pr.x,pr.y,pr.x+l*(float)sin(w-0.5f)*0.2f,pr.y+l*(float)cos(w-0.5f)*0.2f,z);
		NixDrawLine(pd.x,pd.y,pr.x+l*(float)sin(w+0.5f)*0.2f,pr.y+l*(float)cos(w+0.5f)*0.2f,z);
		NixDrawLine(pd.x,pd.y,pr.x+l*(float)sin(w-0.5f)*0.2f,pr.y+l*(float)cos(w-0.5f)*0.2f,z);
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
		NixSetColor(color(1,0,(i==0)?1:0.5f,0));
		NixDrawLine3D(o, o + e[i] * 30 / ed->multi_view_3d->zoom);
	}
}

void ModeModelSkeleton::OnDrawWin(int win, irect dest)
{
	mode_model_mesh_triangle->DrawTrias();

#ifdef USE_MODELS
	// sub models
	foreachi(data->Bone, b, i)
		if (b.model){
			if (SubMode == ModeModelAnimation)
				b.model->Matrix = PointMatrix[i];
			else
				MatrixTranslation(b.model->Matrix, SkeletonGetPointPos(i));
			b.model->OnDraw(0, false, false);
		}
#endif

	NixSetZ(false, false);
	NixEnableLighting(false);
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


