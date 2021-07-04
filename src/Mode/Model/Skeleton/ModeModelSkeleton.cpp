/*
 * ModeModelSkeleton.cpp
 *
 *  Created on: 14.03.2012
 *      Author: michi
 */

#include "ModeModelSkeleton.h"
#include "Creation/ModeModelSkeletonCreateBone.h"
#include "Creation/ModeModelSkeletonAttachVertices.h"
#include "../ModeModel.h"
#include "../Animation/ModeModelAnimation.h"
#include "../Mesh/Selection/MeshSelectionModePolygon.h"
#include "../../../Data/Model/ModelSelection.h"
#include "../../../Edward.h"
#include "../../../Storage/Storage.h"
#include "../../../MultiView/MultiView.h"
#include "../../../MultiView/Window.h"
#include "../../../MultiView/DrawingHelper.h"
#include "../../../MultiView/ColorScheme.h"
#include "../../../lib/nix/nix.h"



ModeModelSkeleton *mode_model_skeleton = NULL;


ModeModelSkeleton::ModeModelSkeleton(ModeBase *_parent) :
	Mode<DataModel>("ModelSkeleton", _parent, ed->multi_view_3d, "menu_skeleton")
{
	mouse_action = -1;
}



void ModeModelSkeleton::on_command(const string & id) {
	if (id == "skeleton_new_point")
		ed->set_mode(new ModeModelSkeletonCreateBone(ed->cur_mode));
	if (id == "skeleton_edit_bone") {
		if (data->get_selection().bone.num == 1) {
			foreachi(ModelBone &b, data->bone, i)
				if (b.is_selected) {
					ed->set_mode(new ModeModelSkeletonAttachVertices(ed->cur_mode, i));
					break;
				}
		} else {
			ed->error_box(_("Please select exactly one bone!"));
		}
	}
	//if (id == "skeleton_link")
	//	ed->setMode(new ModeModelSkeletonCreateBone(ed->cur_mode));
	if (id == "skeleton_unlink")
		unlink_selection();

	if (id == "delete")
		data->deleteSelectedBones();

	if (id == "skeleton_add_model")
		add_sub_model();
	if (id == "skeleton_no_model")
		remove_sub_model();

	if (id == "select")
		choose_mouse_function(MultiView::ACTION_SELECT);
	if (id == "translate")
		choose_mouse_function(MultiView::ACTION_MOVE);
	if (id == "rotate")
		choose_mouse_function(MultiView::ACTION_ROTATE);
	if (id == "scale")
		choose_mouse_function(MultiView::ACTION_SCALE);
	if (id == "mirror")
		choose_mouse_function(MultiView::ACTION_MIRROR);
}

void ModeModelSkeleton::add_sub_model() {
	if (!storage->file_dialog(FD_MODEL, false, true))
		return;
	data->begin_action_group("remove-sub-model");
	foreachi(ModelBone &b, data->bone, i)
		if (b.is_selected)
			data->setBoneModel(i, storage->dialog_file_no_ending);
	data->end_action_group();
}

void ModeModelSkeleton::remove_sub_model() {
	data->begin_action_group("remove-sub-model");
	foreachi(ModelBone &b, data->bone, i)
		if (b.is_selected)
			data->setBoneModel(i, "");
	data->end_action_group();
}

void ModeModelSkeleton::unlink_selection() {
	data->begin_action_group("unlink-bones");
	int n = 0;
	foreachi(ModelBone &b, data->bone, i)
		if ((b.is_selected) and (b.parent >= 0))
			if (data->bone[b.parent].is_selected) {
				data->reconnectBone(i, -1);
				n ++;
			}
	data->end_action_group();

	ed->set_message(format(_("destroyed %d links"), n));
}

void ModeModelSkeleton::choose_mouse_function(int f) {
	mouse_action = f;

	multi_view->set_mouse_action("ActionModelTransformBones", mouse_action, false);
}


void ModeModelSkeleton::on_draw() {
}



void ModeModelSkeleton::on_update_menu() {
	ed->check("select", mouse_action == MultiView::ACTION_SELECT);
	ed->check("translate", mouse_action == MultiView::ACTION_MOVE);
	ed->check("rotate", mouse_action == MultiView::ACTION_ROTATE);
	ed->check("scale", mouse_action == MultiView::ACTION_SCALE);
	ed->check("mirror", mouse_action == MultiView::ACTION_MIRROR);
}



void ModeModelSkeleton::on_start() {
	ed->toolbar[hui::TOOLBAR_LEFT]->set_by_id("model-skeleton-toolbar");

	data->subscribe(this, [=]{
		mode_model_mesh->update_vertex_buffers(data->mesh->vertex);
	});


	choose_mouse_function(MultiView::ACTION_MOVE);
	mode_model->allow_selection_modes(false);
	mode_model_mesh->update_vertex_buffers(data->mesh->vertex);
}



void ModeModelSkeleton::on_end() {
	data->unsubscribe(this);
}


void ModeModelSkeleton::on_set_multi_view() {
	multi_view->clear_data(data);

	multi_view->add_data(MVD_SKELETON_BONE,
			data->bone,
			MultiView::FLAG_DRAW | MultiView::FLAG_INDEX | MultiView::FLAG_SELECT | MultiView::FLAG_MOVE);
}




void draw_bone(const vector &r, const vector &d, const color &c, MultiView::Window *win) {
	Array<color> col;
	col.add(color::interpolate(c, scheme.BACKGROUND, 0.5f)); // root
	col.add(color::interpolate(c, scheme.BACKGROUND, 0.8f));
	draw_lines_colored({r,d}, col, false);
}

void draw_coord_basis(MultiView::Window *win, const ModelBone &b) {
	vector o = b.pos;
	vector e[3] = {vector::EX, vector::EY, vector::EZ};
	if (ed->cur_mode == mode_model_animation)
		for (int i=0;i<3;i++)
			e[i] = b._matrix.transform_normal(e[i]);
	for (int i=0; i<3; i++) {
		color cc = scheme.AXIS[i];
		draw_lines_colored({o, o + e[i] * 40 / win->zoom()}, {cc,cc}, false);
	}
}

void ModeModelSkeleton::on_draw_win(MultiView::Window *win) {
	mode_model_mesh->draw_mesh(win, data->mesh, data->mesh->vertex, false);
	draw_skeleton(win, data->bone);
}

void ModeModelSkeleton::draw_skeleton(MultiView::Window *win, Array<ModelBone> &bone, bool thin) {
	// sub models
	foreachi(ModelBone &b, data->bone, i) {
		if (b.view_stage < multi_view->view_stage)
			continue;
		if (!b.model)
			continue;
		b.model->_matrix = b._matrix;
		//b.model->draw(0, false, false);
		// TODO draw sub models
	}
	nix::set_model_matrix(matrix::ID);

	nix::set_z(false, false);
	set_line_width(thin ? scheme.LINE_WIDTH_THIN : scheme.LINE_WIDTH_BONE);

	for (ModelBone &b: bone) {
		if (b.view_stage < multi_view->view_stage)
			continue;

		if (b.is_selected)
			draw_coord_basis(win, b);
		int r = b.parent;
		if (r < 0)
			continue;
		color c = bone[r].is_selected ? scheme.SELECTION : scheme.POINT;
		if (multi_view->hover.index == r)
			c = color::interpolate(c, scheme.HOVER, 0.3f);
		draw_bone(bone[r].pos, b.pos, c, win);
	}
	nix::set_z(true, true);
}


