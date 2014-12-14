/*
 * ModeModelSkeleton.cpp
 *
 *  Created on: 14.03.2012
 *      Author: michi
 */

#include "../../../Edward.h"
#include "../../../MultiView/MultiView.h"
#include "../../../MultiView/Window.h"
#include "ModeModelSkeleton.h"
#include "../Mesh/ModeModelMeshPolygon.h"
#include "../Animation/ModeModelAnimation.h"
#include "Creation/ModeModelSkeletonCreateBone.h"
#include "Creation/ModeModelSkeletonAttachVertices.h"
#include "../../../Action/Model/Skeleton/ActionModelDeleteBoneSelection.h"


ModeModelSkeleton *mode_model_skeleton = NULL;


ModeModelSkeleton::ModeModelSkeleton(ModeBase *_parent) :
	Mode<DataModel>("ModelSkeleton", _parent, ed->multi_view_3d, "menu_skeleton")
{
}

ModeModelSkeleton::~ModeModelSkeleton()
{
}



void ModeModelSkeleton::onCommand(const string & id)
{
	if (id == "skeleton_new_point")
		ed->setMode(new ModeModelSkeletonCreateBone(ed->cur_mode));
	if (id == "skeleton_edit_bone"){
		if (data->GetNumSelectedBones() == 1){
			foreachi(ModelBone &b, data->Bone, i)
				if (b.is_selected){
					ed->setMode(new ModeModelSkeletonAttachVertices(ed->cur_mode, i));
					break;
				}
		}else{
			ed->errorBox(_("Es muss genau 1 Knochen markiert sein!"));
		}
	}

	if (id == "delete")
		data->execute(new ActionModelDeleteBoneSelection(data));

	if (id == "select")
		chooseMouseFunction(MultiView::ActionSelect);
	if (id == "translate")
		chooseMouseFunction(MultiView::ActionMove);
	if (id == "rotate")
		chooseMouseFunction(MultiView::ActionRotate);
	if (id == "scale")
		chooseMouseFunction(MultiView::ActionScale);
	if (id == "mirror")
		chooseMouseFunction(MultiView::ActionMirror);
}


void ModeModelSkeleton::chooseMouseFunction(int f)
{
	mouse_action = f;
	ed->updateMenu();

	// mouse action
	if (mouse_action != MultiView::ActionSelect){
		multi_view->SetMouseAction("ActionModelAnimationTransformBones", mouse_action);
	}else{
		multi_view->SetMouseAction("", MultiView::ActionSelect);
	}
}


void ModeModelSkeleton::onDraw()
{
}



void ModeModelSkeleton::onUpdateMenu()
{
	ed->check("select", mouse_action == MultiView::ActionSelect);
	ed->check("translate", mouse_action == MultiView::ActionMove);
	ed->check("rotate", mouse_action == MultiView::ActionRotate);
	ed->check("scale", mouse_action == MultiView::ActionScale);
	ed->check("mirror", mouse_action == MultiView::ActionMirror);
}



void ModeModelSkeleton::onStart()
{
	string dir = (HuiAppDirectoryStatic + "Data/icons/toolbar/").sys_filename();
	HuiToolbar *t = ed->toolbar[HuiToolbarLeft];
	t->reset();
	t->addSeparator();
	t->addItemCheckable(_("Selektieren"), dir + "rf_select.png", "select");
	t->addItemCheckable(_("Verschieben"), dir + "rf_translate.png", "translate");
	t->addItemCheckable(_("Rotieren"), dir + "rf_rotate.png", "rotate");
	t->addItemCheckable(_("Skalieren"), dir + "rf_scale.png", "scale");
	t->addItemCheckable(_("Spiegeln"),dir + "rf_mirror.png", "mirror");
	t->enable(true);
	t->configure(false,true);

	// relative to absolute pos
	foreach(ModelBone &b, data->Bone)
		if (b.Parent >= 0)
			b.pos = data->Bone[b.Parent].pos + b.DeltaPos;
		else
			b.pos = b.DeltaPos;

	subscribe(data);
	subscribe(multi_view, multi_view->MESSAGE_SELECTION_CHANGE);

	multi_view->ClearData(data);
	multi_view->SetAllowRect(true);

	chooseMouseFunction(MultiView::ActionSelectAndMove);


	// left -> translate
	multi_view->SetMouseAction("ActionModelTransformBones", MultiView::ActionSelectAndMove);
	multi_view->allow_rect = true;
	onUpdate(data, "");
}



void ModeModelSkeleton::onEnd()
{
	multi_view->ClearData(NULL);
	unsubscribe(data);
	unsubscribe(multi_view);
}



void ModeModelSkeleton::onUpdate(Observable *o, const string &message)
{
	if (o == data){

		multi_view->ClearData(data);

		//CModeAll::SetMultiViewViewStage(&ViewStage, false);
		multi_view->AddData(	MVDSkeletonPoint,
				data->Bone,
				NULL,
				MultiView::FlagDraw | MultiView::FlagIndex | MultiView::FlagSelect | MultiView::FlagMove);
	}else if (o == multi_view){
	}
}



void DrawBone(const vector &r, const vector &d, const color &c, MultiView::Window *win)
{
	vector pr = win->project(r);
	vector pd = win->project(d);
	if ((pr.z>0)&&(pd.z>0)&&(pr.z<1)&&(pd.z<1)){
		float z=(pr.z+pd.z)/2;
		pr.z=pd.z=0;
		vector d=pr-pd;
		//NixDrawLine(pr.x,pr.y,pd.x,pd.y,c,z);
		float l=d.length();
		float w=(float)atan2(d.x,d.y)+pi;
		NixSetColor(c);
		NixDrawLine(pr.x,pr.y,pr.x+l*(float)sin(w+0.5f)*0.2f,pr.y+l*(float)cos(w+0.5f)*0.2f,z);
		NixDrawLine(pr.x,pr.y,pr.x+l*(float)sin(w-0.5f)*0.2f,pr.y+l*(float)cos(w-0.5f)*0.2f,z);
		NixDrawLine(pd.x,pd.y,pr.x+l*(float)sin(w+0.5f)*0.2f,pr.y+l*(float)cos(w+0.5f)*0.2f,z);
		NixDrawLine(pd.x,pd.y,pr.x+l*(float)sin(w-0.5f)*0.2f,pr.y+l*(float)cos(w-0.5f)*0.2f,z);
	}
}

void DrawCoordBasis(const ModelBone *b)
{
	vector o = b->pos;
	vector e[3];
	e[0] = e_x;
	e[1] = e_y;
	e[2] = e_z;
	if (ed->cur_mode == mode_model_animation)
		for (int i=0;i<3;i++)
			e[i] = b->Matrix.transform_normal(e[i]);
	for (int i=0;i<3;i++){
		NixSetColor(color(1,0,(i==0)?1:0.5f,0));
		NixDrawLine3D(o, o + e[i] * 30 / ed->multi_view_3d->cam.zoom);
	}
}

void ModeModelSkeleton::onDrawWin(MultiView::Window *win)
{
	if (mode_model_animation->isAncestorOf(ed->cur_mode))
		mode_model_mesh_polygon->DrawPolygons(win, mode_model_animation->vertex);
	else
		mode_model_mesh_polygon->DrawPolygons(win, data->Vertex);

#ifdef USE_MODELS
	// sub models
	foreachi(data->Bone, b, i)
		if (b.model){
			if (SubMode == ModeModelAnimation)
				b.model->Matrix = PointMatrix[i];
			else
				MatrixTranslation(b.model->Matrix, SkeletonGetPointPos(i));
			b.model->onDraw(0, false, false);
		}
#endif

	NixSetZ(false, false);
	NixEnableLighting(false);
	NixSetWire(false);

	foreach(ModelBone &b, data->Bone){
		/*if (b.view_stage<=ViewStage)
			continue;*/
		if (b.is_selected)
			DrawCoordBasis(&b);
		int r = b.Parent;
		if (r < 0)
			continue;
		color c = data->Bone[r].is_selected ? Red : Blue;
		if (multi_view->hover.index == r)
			c = ColorInterpolate(c, White, 0.3f);
		DrawBone(data->Bone[r].pos, b.pos, c, win);
	}
	NixSetZ(true, true);
}


