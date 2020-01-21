/*
 * ModeModelSkeleton.cpp
 *
 *  Created on: 14.03.2012
 *      Author: michi
 */

#include "../../../Edward.h"
#include "../../../Storage/Storage.h"
#include "../../../MultiView/MultiView.h"
#include "../../../MultiView/Window.h"
#include "../../../MultiView/DrawingHelper.h"
#include "../../../MultiView/ColorScheme.h"
#include "../../../lib/nix/nix.h"
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



void ModeModelSkeleton::on_command(const string & id)
{
	if (id == "skeleton_new_point")
		ed->set_mode(new ModeModelSkeletonCreateBone(ed->cur_mode));
	if (id == "skeleton_edit_bone"){
		if (data->getNumSelectedBones() == 1){
			foreachi(ModelBone &b, data->bone, i)
				if (b.is_selected){
					ed->set_mode(new ModeModelSkeletonAttachVertices(ed->cur_mode, i));
					break;
				}
		}else{
			ed->error_box(_("Please select exactly one bone!"));
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
	if (!storage->file_dialog(FD_MODEL, false, true))
		return;
	data->begin_action_group("remove-sub-model");
	foreachi(ModelBone &b, data->bone, i)
		if (b.is_selected)
			data->setBoneModel(i, storage->dialog_file_no_ending);
	data->end_action_group();
}

void ModeModelSkeleton::removeSubModel()
{
	data->begin_action_group("remove-sub-model");
	foreachi(ModelBone &b, data->bone, i)
		if (b.is_selected)
			data->setBoneModel(i, "");
	data->end_action_group();
}

void ModeModelSkeleton::unlinkSelection()
{
	data->begin_action_group("unlink-bones");
	int n = 0;
	foreachi(ModelBone &b, data->bone, i)
		if ((b.is_selected) and (b.parent >= 0))
			if (data->bone[b.parent].is_selected){
				data->reconnectBone(i, -1);
				n ++;
			}
	data->end_action_group();

	ed->set_message(format(_("destroyed %d links"), n));
}

void ModeModelSkeleton::chooseMouseFunction(int f)
{
	mouse_action = f;

	multi_view->set_mouse_action("ActionModelTransformBones", mouse_action, false);
}


void ModeModelSkeleton::on_draw()
{
}



void ModeModelSkeleton::on_update_menu()
{
	ed->check("select", mouse_action == MultiView::ACTION_SELECT);
	ed->check("translate", mouse_action == MultiView::ACTION_MOVE);
	ed->check("rotate", mouse_action == MultiView::ACTION_ROTATE);
	ed->check("scale", mouse_action == MultiView::ACTION_SCALE);
	ed->check("mirror", mouse_action == MultiView::ACTION_MIRROR);
}



void ModeModelSkeleton::on_start()
{
	ed->toolbar[hui::TOOLBAR_LEFT]->set_by_id("model-skeleton-toolbar");

	subscribe(data);


	chooseMouseFunction(MultiView::ACTION_MOVE);
	mode_model->allow_selection_modes(false);
	on_update(data, "");
}



void ModeModelSkeleton::on_end()
{
	unsubscribe(data);
}


void ModeModelSkeleton::on_set_multi_view()
{
	multi_view->clear_data(data);

	//CModeAll::SetMultiViewViewStage(&ViewStage, false);
	multi_view->add_data(	MVD_SKELETON_BONE,
			data->bone,
			NULL,
			MultiView::FLAG_DRAW | MultiView::FLAG_INDEX | MultiView::FLAG_SELECT | MultiView::FLAG_MOVE);
}


void ModeModelSkeleton::on_update(Observable *o, const string &message)
{
	if (o == data){
		mode_model_mesh->update_vertex_buffers(data->vertex);
	}
}



void drawBone(const vector &r, const vector &d, const color &c, MultiView::Window *win)
{
	Array<color> col;
	col.add(ColorInterpolate(c, scheme.BACKGROUND, 0.5f)); // root
	col.add(ColorInterpolate(c, scheme.BACKGROUND, 0.8f));
	nix::DrawLinesColored({r,d}, col, false);
}

void drawCoordBasis(MultiView::Window *win, const ModelBone &b)
{
	vector o = b.pos;
	vector e[3] = {vector::EX, vector::EY, vector::EZ};
	if (ed->cur_mode == mode_model_animation)
		for (int i=0;i<3;i++)
			e[i] = b._matrix.transform_normal(e[i]);
	for (int i=0;i<3;i++){
		color cc = color(1,0,(i==0)?1:0.5f,0);
		nix::DrawLinesColored({o, o + e[i] * 30 / win->zoom()}, {cc,cc}, false);
	}
}

void ModeModelSkeleton::on_draw_win(MultiView::Window *win)
{
	mode_model_mesh->draw_polygons(win, data->vertex);
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
	nix::SetWorldMatrix(matrix::ID);

	nix::SetZ(false, false);
	nix::EnableLighting(false);
	nix::SetWire(false);
	set_wide_lines(thin ? scheme.LINE_WIDTH_THIN : scheme.LINE_WIDTH_BONE);

	for (ModelBone &b: bone){
		if (b.view_stage < multi_view->view_stage)
			continue;

		if (b.is_selected)
			drawCoordBasis(win, b);
		int r = b.parent;
		if (r < 0)
			continue;
		color c = bone[r].is_selected ? scheme.SELECTION : scheme.POINT;
		if (multi_view->hover.index == r)
			c = ColorInterpolate(c, scheme.HOVER, 0.3f);
		drawBone(bone[r].pos, b.pos, c, win);
	}
	nix::SetZ(true, true);
}


