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
#include "../ModeModel.h"
#include "../Animation/ModeModelAnimation.h"
#include "Creation/ModeModelSkeletonCreateBone.h"
#include "Creation/ModeModelSkeletonAttachVertices.h"
#include "../Mesh/Selection/MeshSelectionModePolygon.h"



ModeModelSkeleton *mode_model_skeleton = NULL;


ModeModelSkeleton::ModeModelSkeleton(ModeBase *_parent) :
	Mode<DataModel>("ModelSkeleton", _parent, ed->multi_view_3d, "menu_skeleton")
{
	mouse_action = -1;
}



void ModeModelSkeleton::onCommand(const string & id)
{
	if (id == "skeleton_new_point")
		ed->setMode(new ModeModelSkeletonCreateBone(ed->cur_mode));
	if (id == "skeleton_edit_bone"){
		if (data->getNumSelectedBones() == 1){
			foreachi(ModelBone &b, data->bone, i)
				if (b.is_selected){
					ed->setMode(new ModeModelSkeletonAttachVertices(ed->cur_mode, i));
					break;
				}
		}else{
			ed->errorBox(_("Es muss genau 1 Knochen markiert sein!"));
		}
	}
	//if (id == "skeleton_link")
	//	ed->setMode(new ModeModelSkeletonCreateBone(ed->cur_mode));
	if (id == "skeleton_unlink")
		unlinkSelection();

	if (id == "delete")
		data->deleteSelectedBones();

	if (id == "skeleton_add_model")
		addSubModel();
	if (id == "skeleton_no_model")
		removeSubModel();

	if (id == "select")
		chooseMouseFunction(MultiView::ACTION_SELECT);
	if (id == "translate")
		chooseMouseFunction(MultiView::ACTION_MOVE);
	if (id == "rotate")
		chooseMouseFunction(MultiView::ACTION_ROTATE);
	if (id == "scale")
		chooseMouseFunction(MultiView::ACTION_SCALE);
	if (id == "mirror")
		chooseMouseFunction(MultiView::ACTION_MIRROR);
}

void ModeModelSkeleton::addSubModel()
{
	if (!ed->fileDialog(FD_MODEL, false, true))
		return;
	data->beginActionGroup("remove-sub-model");
	foreachi(ModelBone &b, data->bone, i)
		if (b.is_selected)
			data->setBoneModel(i, ed->dialog_file_no_ending);
	data->endActionGroup();
}

void ModeModelSkeleton::removeSubModel()
{
	data->beginActionGroup("remove-sub-model");
	foreachi(ModelBone &b, data->bone, i)
		if (b.is_selected)
			data->setBoneModel(i, "");
	data->endActionGroup();
}

void ModeModelSkeleton::unlinkSelection()
{
	data->beginActionGroup("unlink-bones");
	int n = 0;
	foreachi(ModelBone &b, data->bone, i)
		if ((b.is_selected) and (b.parent >= 0))
			if (data->bone[b.parent].is_selected){
				data->reconnectBone(i, -1);
				n ++;
			}
	data->endActionGroup();

	ed->setMessage(format(_("%d Verbindungen gel&ost"), n));
}

void ModeModelSkeleton::chooseMouseFunction(int f)
{
	mouse_action = f;

	multi_view->setMouseAction("ActionModelTransformBones", mouse_action, false);
}


void ModeModelSkeleton::onDraw()
{
}



void ModeModelSkeleton::onUpdateMenu()
{
	ed->check("select", mouse_action == MultiView::ACTION_SELECT);
	ed->check("translate", mouse_action == MultiView::ACTION_MOVE);
	ed->check("rotate", mouse_action == MultiView::ACTION_ROTATE);
	ed->check("scale", mouse_action == MultiView::ACTION_SCALE);
	ed->check("mirror", mouse_action == MultiView::ACTION_MIRROR);
}



void ModeModelSkeleton::onStart()
{
	string dir = (app->directory_static + "Data/icons/toolbar/").sys_filename();
	hui::Toolbar *t = ed->toolbar[hui::TOOLBAR_LEFT];
	t->reset();
	t->addItemCheckable(_("neuer Knochen"),dir + "new_vertex.png", "skeleton_new_point");
	t->addSeparator();
	t->addItemCheckable(_("Selektieren"), dir + "rf_select.png", "select");
	t->addItemCheckable(_("Verschieben"), dir + "rf_translate.png", "translate");
	t->addItemCheckable(_("Rotieren"), dir + "rf_rotate.png", "rotate");
	t->addItemCheckable(_("Skalieren"), dir + "rf_scale.png", "scale");
	t->addItemCheckable(_("Spiegeln"),dir + "rf_mirror.png", "mirror");
	t->enable(true);
	t->configure(false,true);

	subscribe(data);
	subscribe(multi_view, multi_view->MESSAGE_SELECTION_CHANGE);

	multi_view->clearData(data);
	multi_view->setAllowSelect(true);

	chooseMouseFunction(MultiView::ACTION_MOVE);
	mode_model->allowSelectionModes(false);
	onUpdate(data, "");
}



void ModeModelSkeleton::onEnd()
{
	unsubscribe(data);
	unsubscribe(multi_view);
}



void ModeModelSkeleton::onUpdate(Observable *o, const string &message)
{
	if (o == data){
		mode_model_mesh->updateVertexBuffers(data->vertex);

		multi_view->clearData(data);

		//CModeAll::SetMultiViewViewStage(&ViewStage, false);
		multi_view->addData(	MVD_SKELETON_BONE,
				data->bone,
				NULL,
				MultiView::FLAG_DRAW | MultiView::FLAG_INDEX | MultiView::FLAG_SELECT | MultiView::FLAG_MOVE);
	}else if (o == multi_view){
	}
}



void drawBone(const vector &r, const vector &d, const color &c, MultiView::Window *win)
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
		nix::SetColor(c);
		nix::DrawLine(pr.x,pr.y,pr.x+l*(float)sin(w+0.5f)*0.2f,pr.y+l*(float)cos(w+0.5f)*0.2f,z);
		nix::DrawLine(pr.x,pr.y,pr.x+l*(float)sin(w-0.5f)*0.2f,pr.y+l*(float)cos(w-0.5f)*0.2f,z);
		nix::DrawLine(pd.x,pd.y,pr.x+l*(float)sin(w+0.5f)*0.2f,pr.y+l*(float)cos(w+0.5f)*0.2f,z);
		nix::DrawLine(pd.x,pd.y,pr.x+l*(float)sin(w-0.5f)*0.2f,pr.y+l*(float)cos(w-0.5f)*0.2f,z);
	}
}

void drawCoordBasis(MultiView::Window *win, const ModelBone &b)
{
	vector o = b.pos;
	vector e[3] = {e_x, e_y, e_z};
	if (ed->cur_mode == mode_model_animation)
		for (int i=0;i<3;i++)
			e[i] = b._matrix.transform_normal(e[i]);
	for (int i=0;i<3;i++){
		nix::SetColor(color(1,0,(i==0)?1:0.5f,0));
		nix::DrawLine3D(o, o + e[i] * 30 / win->zoom());
	}
}

void ModeModelSkeleton::onDrawWin(MultiView::Window *win)
{
	mode_model_mesh->drawPolygons(win, data->vertex);
	drawSkeleton(win, data->bone);
}

void ModeModelSkeleton::drawSkeleton(MultiView::Window *win, Array<ModelBone> &bone, bool thin)
{
	// sub models
	foreachi(ModelBone &b, data->bone, i){
		if (b.view_stage < multi_view->view_stage)
			continue;
		if (!b.model)
			continue;
		b.model->_matrix = b._matrix;
		b.model->Draw(0, false, false);
	}
	nix::SetWorldMatrix(m_id);

	nix::SetZ(false, false);
	nix::EnableLighting(false);
	nix::SetWire(false);
	nix::line_width = thin ? 0.5f : 2;
	nix::smooth_lines = true;

	for (ModelBone &b: bone){
		if (b.view_stage < multi_view->view_stage)
			continue;

		if (b.is_selected)
			drawCoordBasis(win, b);
		int r = b.parent;
		if (r < 0)
			continue;
		color c = bone[r].is_selected ? Red : Blue;
		if (multi_view->hover.index == r)
			c = ColorInterpolate(c, White, 0.3f);
		drawBone(bone[r].pos, b.pos, c, win);
	}
	nix::SetZ(true, true);
	nix::line_width = 1;
	nix::smooth_lines = false;
}


