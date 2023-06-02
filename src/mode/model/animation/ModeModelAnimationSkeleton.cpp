/*
 * ModeModelAnimationSkeleton.cpp
 *
 *  Created on: 16.08.2012
 *      Author: michi
 */

#include "ModeModelAnimationSkeleton.h"
#include "../skeleton/ModeModelSkeleton.h"
#include "../animation/ModeModelAnimation.h"
#include "../mesh/ModeModelMesh.h"
#include "../ModeModel.h"
#include "../../../Edward.h"
#include "../../../multiview/MultiView.h"
#include "../../../multiview/DrawingHelper.h"
#include "../../../lib/nix/nix.h"
#include "../../../data/model/ModelSelection.h"

ModeModelAnimationSkeleton *mode_model_animation_skeleton = NULL;

float poly_hover(ModelPolygon *pol, MultiView::Window *win, const vec2 &M, vec3 &tp, float &z, const Array<ModelVertex> &vertex);

ModeModelAnimationSkeleton::ModeModelAnimationSkeleton(ModeBase* _parent, MultiView::MultiView *mv) :
	Mode<DataModel>("ModelAnimationSkeleton", _parent, mv, "menu_move"),
	current_move(((ModeModelAnimation*)parent)->current_move),
	current_frame(((ModeModelAnimation*)parent)->current_frame)
{
	select_recursive = true;
	mouse_action = -1;
}

void ModeModelAnimationSkeleton::on_start() {
	ed->get_toolbar(hui::TOOLBAR_LEFT)->set_by_id("model-animation-skeleton-toolbar");

	foreachi(ModelBone &b, data->bone, i)
		mode_model_animation->bone[i].is_selected = b.is_selected;

	chooseMouseFunction(MultiView::ACTION_ROTATE);

	multi_view->subscribe(this, [=]{ updateSelection(); }, multi_view->MESSAGE_SELECTION_CHANGE);
}

void ModeModelAnimationSkeleton::on_end() {
	multi_view->unsubscribe(this);
}

void ModeModelAnimationSkeleton::on_command(const string& id) {
	if (id == "select")
		chooseMouseFunction(MultiView::ACTION_SELECT);
	if (id == "translate")
		chooseMouseFunction(MultiView::ACTION_MOVE);
	if (id == "rotate")
		chooseMouseFunction(MultiView::ACTION_ROTATE);
	if (id == "select-recursive") {
		select_recursive = !select_recursive;
		updateSelection();
		ed->update_menu();
	}
	if (id == "copy")
		copy();
	if (id == "paste")
		paste();
}

void ModeModelAnimationSkeleton::chooseMouseFunction(int f) {
	mouse_action = f;

	// mouse action
	multi_view->set_mouse_action("ActionModelAnimationTransformBones", mouse_action, false);
}


float bone_hover(const MultiView::SingleData *pp, MultiView::Window *win, const vec2 &m, vec3 &tp, float &z, ModeModelAnimationSkeleton *me) {
	float dmin = 100;
	for (auto &p: me->data->mesh->polygon) {
		if (pp == &mode_model_animation->bone[me->data->mesh->vertex[p.side[0].vertex].bone_index.i]) {
			float d = poly_hover(&p, win, m, tp, z, mode_model_animation->vertex);
			if (d >= 0 and d < dmin)
				dmin = d;
		}
	}
	if (dmin == 100)
		return -1;
	return dmin;
}

void ModeModelAnimationSkeleton::on_set_multi_view() {
	multi_view->clear_data(data);
	//CModeAll::SetMultiViewViewStage(&ViewStage, false);

	multi_view->add_data(MVD_SKELETON_BONE,
			mode_model_animation->bone,
			MultiView::FLAG_DRAW | MultiView::FLAG_INDEX | MultiView::FLAG_SELECT);
	multi_view->set_hover_func(MVD_SKELETON_BONE, [=](const MultiView::SingleData *pp, MultiView::Window *win, const vec2 &m, vec3 &tp, float &z){ return bone_hover(pp, win, m, tp, z, this); });
}

void ModeModelAnimationSkeleton::on_draw_win(MultiView::Window *win) {
	mode_model_mesh->draw_mesh(win, data->mesh, mode_model_animation->vertex, false);
	mode_model_skeleton->draw_skeleton(win, mode_model_animation->bone, true);


	if ((multi_view->hover.index < 0) or (multi_view->hover.type != MVD_SKELETON_BONE))
		return;

	VertexStagingBuffer vbs;
	for (ModelPolygon &p: data->mesh->polygon)
		if (data->mesh->vertex[p.side[0].vertex].bone_index.i == multi_view->hover.index)
			p.add_to_vertex_buffer(mode_model_animation->vertex, vbs, 1);
	vbs.build(mode_model_mesh->vb_hover, 1);


	nix::set_offset(-1.0f);
	set_material_hover();
	nix::draw_triangles(mode_model_mesh->vb_hover);
	nix::set_material(White, 0.5f, 0, Black);
	nix::disable_alpha();
	nix::set_offset(0);
}



void ModeModelAnimationSkeleton::on_update_menu() {
	ed->check("select", mouse_action == MultiView::ACTION_SELECT);
	ed->check("translate", mouse_action == MultiView::ACTION_MOVE);
	ed->check("rotate", mouse_action == MultiView::ACTION_ROTATE);

	ed->check("select-recursive", select_recursive);
}

void ModeModelAnimationSkeleton::updateSelection() {
	foreachi(auto &b, data->bone, i)
		b.is_selected = mode_model_animation->bone[i].is_selected;

	// also select children?
	if (select_recursive) {
		// this works thanks to lucky circumstances... might break in the future
		foreachi(auto &b, data->bone, i)
			if (b.is_selected) {
				for (auto &bc: data->bone)
					if (bc.parent == i)
						bc.is_selected = true;
			}
	}


	// select geometry
	for (auto &v: data->mesh->vertex)
		v.is_selected = data->bone[v.bone_index.i].is_selected;
	data->selectionFromVertices();

	mode_model_mesh->fill_selection_buffer(mode_model_animation->vertex);
}

void ModeModelAnimationSkeleton::copy() {
	int n = data->get_selection().bone.num;
	if (n == 0) {
		ed->set_message(_("nothing selected"));
		return;
	} else if (n == 1) {
		temp.clear();
		ModelFrame f;
		foreachi(auto &b, data->bone, i)
			if (b.is_selected) {
				f.skel_ang.add(mode_model_animation->cur_move()->frame[current_frame].skel_ang[i]);
				f.skel_dpos.add(mode_model_animation->cur_move()->frame[current_frame].skel_dpos[i]);
			}
		temp.add(f);
		ed->set_message(format(_("Animation copied from single bone"), n));
	} else {
		temp.clear();
		temp.add(mode_model_animation->cur_move()->frame[current_frame]);
		ed->set_message(format(_("copied animation of %d bones"), data->bone.num));
	}
}

void ModeModelAnimationSkeleton::paste() {
	if (temp.num == 0) {
		ed->set_message(_("Clipboard is empty"));
		return;
	}
	data->begin_action_group("paste-animation");
	int n = data->get_selection().bone.num;
	int nt = temp[0].skel_ang.num;
	if (nt == 1) {
		foreachi(auto &b, data->bone, i)
			if (b.is_selected)
				data->animationSetBone(current_move, current_frame, i, temp[0].skel_dpos[0], temp[0].skel_ang[0]);
		if (n == 1)
			ed->set_message(_("Animation pasted - single bone"));
		else
			ed->set_message(_("Animation pasted - single bone onto multiple"));
	} else {
		foreachi(auto &b, data->bone, i)
			if (b.is_selected)
				data->animationSetBone(current_move, current_frame, i, temp[0].skel_dpos[i], temp[0].skel_ang[i]);
		ed->set_message(format(_("Animation inserted - on %d bones"), n));
	}
	data->end_action_group();
}
