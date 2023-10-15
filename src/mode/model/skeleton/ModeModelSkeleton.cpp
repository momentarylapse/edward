/*
 * ModeModelSkeleton.cpp
 *
 *  Created on: 14.03.2012
 *      Author: michi
 */

#include "ModeModelSkeleton.h"
#include "creation/ModeModelSkeletonCreateBone.h"
#include "creation/ModeModelSkeletonAttachVertices.h"
#include "../ModeModel.h"
#include "../animation/ModeModelAnimation.h"
#include "../mesh/selection/MeshSelectionModePolygon.h"
#include "../../../data/model/ModelSelection.h"
#include "../../../data/model/ModelMesh.h"
#include "../../../EdwardWindow.h"
#include "../../../Session.h"
#include "../../../storage/Storage.h"
#include "../../../multiview/MultiView.h"
#include "../../../multiview/Window.h"
#include "../../../multiview/DrawingHelper.h"
#include "../../../multiview/ColorScheme.h"
#include "../../../lib/nix/nix.h"



void draw_model(MultiView::Window *win, Model *m, int num_lights); // ModeWorld.cpp


ModeModelSkeleton::ModeModelSkeleton(ModeModel *_parent, MultiView::MultiView *mv) :
	Mode<ModeModel, DataModel>(_parent->session, "ModelSkeleton", _parent, mv, "menu_skeleton")
{
	mode_model_mesh = _parent->mode_model_mesh;
	mouse_action = -1;
}



void ModeModelSkeleton::on_command(const string & id) {
	if (id == "skeleton_new_point")
		session->set_mode(new ModeModelSkeletonCreateBone(this));
	if (id == "skeleton_edit_bone") {
		if (data->get_selection().bone.num == 1) {
			foreachi(ModelBone &b, data->bone, i)
				if (b.is_selected) {
					session->set_mode(new ModeModelSkeletonAttachVertices(this, i));
					break;
				}
		} else {
			session->error(_("Please select exactly one bone!"));
		}
	}
	//if (id == "skeleton_link")
	//	ed->setMode(new ModeModelSkeletonCreateBone(session->cur_mode));
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
	session->storage->file_dialog(FD_MODEL, false, true).then([this] (const auto& p) {
		data->begin_action_group("remove-sub-model");
		foreachi(ModelBone &b, data->bone, i)
			if (b.is_selected)
				data->setBoneModel(i, p.simple);
		data->end_action_group();
	});
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

	session->set_message(format(_("destroyed %d links"), n));
}

void ModeModelSkeleton::choose_mouse_function(int f) {
	mouse_action = f;

	multi_view->set_mouse_action("ActionModelTransformBones", mouse_action, false);
}


void ModeModelSkeleton::on_draw() {
}



void ModeModelSkeleton::on_update_menu() {
	session->win->check("select", mouse_action == MultiView::ACTION_SELECT);
	session->win->check("translate", mouse_action == MultiView::ACTION_MOVE);
	session->win->check("rotate", mouse_action == MultiView::ACTION_ROTATE);
	session->win->check("scale", mouse_action == MultiView::ACTION_SCALE);
	session->win->check("mirror", mouse_action == MultiView::ACTION_MIRROR);
}



void ModeModelSkeleton::on_start() {
	session->win->get_toolbar(hui::TOOLBAR_LEFT)->set_by_id("model-skeleton-toolbar");

	data->out_changed >> create_sink([=]{
		mode_model_mesh->update_vertex_buffers(data->mesh->vertex);
	});


	choose_mouse_function(MultiView::ACTION_MOVE);
	parent->allow_selection_modes(false);
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




void draw_bone(const vec3 &r, const vec3 &d, const color &c, MultiView::Window *win) {
	Array<color> col;
	col.add(color::interpolate(c, scheme.BACKGROUND, 0.5f)); // root
	col.add(color::interpolate(c, scheme.BACKGROUND, 0.8f));
	win->drawing_helper->draw_lines_colored({r,d}, col, false);
}

mat4 get_bone_frame(ModeBase *cur_mode, const ModelBone &b) {
	if (cur_mode->session->mode_model->mode_model_animation->is_ancestor_of(cur_mode))
		return b._matrix;
	return mat4::translation(b.pos);
}

void draw_coord_basis(MultiView::Window *win, const ModelBone &b) {
	vec3 o = b.pos;
	mat4 m = get_bone_frame(win->multi_view->session->cur_mode, b);
	vec3 e[3] = {vec3::EX, vec3::EY, vec3::EZ};
	for (int i=0;i<3;i++)
		e[i] = m.transform_normal(e[i]);
	for (int i=0; i<3; i++) {
		color cc = scheme.AXIS[i];
		win->drawing_helper->draw_lines_colored({o, o + e[i] * 40 / win->zoom()}, {cc,cc}, false);
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
		nix::set_model_matrix(get_bone_frame(win->multi_view->session->cur_mode, b));
		draw_model(win, b.model, 1);
	}
	nix::set_model_matrix(mat4::ID);

	nix::set_z(false, false);
	win->drawing_helper->set_line_width(thin ? scheme.LINE_WIDTH_THIN : scheme.LINE_WIDTH_BONE);

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


