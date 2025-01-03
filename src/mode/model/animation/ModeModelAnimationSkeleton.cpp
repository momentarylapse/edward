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
#include "../../../Session.h"
#include "../../../EdwardWindow.h"
#include "../../../multiview/MultiView.h"
#include "../../../multiview/Window.h"
#include "../../../multiview/DrawingHelper.h"
#include "../../../lib/nix/nix.h"
#include "../../../data/model/ModelSelection.h"
#include "../../../data/model/DataModel.h"
#include "../../../data/model/ModelMesh.h"
#include "../../../data/model/ModelPolygon.h"

float poly_hover(ModelPolygon *pol, MultiView::Window *win, const vec2 &M, vec3 &tp, float &z, const Array<ModelVertex> &vertex);

ModeModelAnimationSkeleton::ModeModelAnimationSkeleton(ModeModelAnimation* _parent, MultiView::MultiView *mv) :
	Mode<ModeModelAnimation, DataModel>(_parent->session, "ModelAnimationSkeleton", _parent, mv, "menu_move"),
	current_move(parent->current_move),
	current_frame(parent->current_frame)
{
	select_recursive = true;
	mouse_action = -1;
}

void ModeModelAnimationSkeleton::on_start() {
	session->win->get_toolbar(hui::TOOLBAR_LEFT)->set_by_id("model-animation-skeleton-toolbar");

	foreachi(ModelBone &b, data->bone, i)
		parent->bone[i].is_selected = b.is_selected;

	chooseMouseFunction(MultiView::ACTION_ROTATE);

	//multi_view->out_selection_changed >> create_sink([=]{ updateSelection(); });
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
		on_selection_change();
		session->win->update_menu();
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

int dominant_bone(const ModelVertex& v) {
	if (v.bone_weight[0] > 0.5)
		return v.bone_index[0];
	if (v.bone_weight[1] > 0.5)
		return v.bone_index[1];
	if (v.bone_weight[2] > 0.5)
		return v.bone_index[2];
	return v.bone_index[3];
}

float bone_hover(const MultiView::SingleData *pp, MultiView::Window *win, const vec2 &m, vec3 &tp, float &z, ModeModelAnimationSkeleton *me) {
	float dmin = 100;
	int index = -1;
	for (auto &p: me->data->mesh->polygon) {
		if (pp == &me->parent->bone[dominant_bone(me->data->mesh->vertex[p.side[0].vertex])]) {
			index = dominant_bone(me->data->mesh->vertex[p.side[0].vertex]);
			float d = poly_hover(&p, win, m, tp, z, me->parent->vertex);
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
			parent->bone,
			MultiView::FLAG_DRAW | MultiView::FLAG_INDEX | MultiView::FLAG_SELECT);
	multi_view->set_hover_func(MVD_SKELETON_BONE, [=](const MultiView::SingleData *pp, MultiView::Window *win, const vec2 &m, vec3 &tp, float &z){ return bone_hover(pp, win, m, tp, z, this); });
}

void ModeModelAnimationSkeleton::on_draw_win(MultiView::Window *win) {
	session->mode_model->mode_model_mesh->draw_mesh(win, data->mesh, parent->vertex, true);
	session->mode_model->mode_model_skeleton->draw_skeleton(win, parent->bone, true);


	if ((multi_view->hover.index < 0) or (multi_view->hover.type != MVD_SKELETON_BONE))
		return;

	VertexStagingBuffer vbs;
	for (ModelPolygon &p: data->mesh->polygon)
		if (dominant_bone(data->mesh->vertex[p.side[0].vertex]) == multi_view->hover.index)
			p.add_to_vertex_buffer(parent->vertex, vbs, 1);
	vbs.build(session->mode_model->mode_model_mesh->vb_hover, 1);


#if HAS_LIB_GL
	nix::set_offset(-1.0f);
	win->drawing_helper->set_material_hover();
	nix::draw_triangles(session->mode_model->mode_model_mesh->vb_hover);
	nix::set_material(White, 0.5f, 0, Black);
	nix::disable_alpha();
	nix::set_offset(0);
#endif
}



void ModeModelAnimationSkeleton::on_update_menu() {
	session->win->check("select", mouse_action == MultiView::ACTION_SELECT);
	session->win->check("translate", mouse_action == MultiView::ACTION_MOVE);
	session->win->check("rotate", mouse_action == MultiView::ACTION_ROTATE);

	session->win->check("select-recursive", select_recursive);
}

void ModeModelAnimationSkeleton::on_selection_change() {
	foreachi(auto &b, data->bone, i)
		b.is_selected = parent->bone[i].is_selected;

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
	for (auto &v: data->mesh->vertex) {
		int b = dominant_bone(v);
		if (b >= 0)
			v.is_selected = data->bone[b].is_selected;
	}
	data->selectionFromVertices();

	session->mode_model->mode_model_mesh->fill_selection_buffer(parent->vertex);
}

void ModeModelAnimationSkeleton::copy() {
	int n = data->get_selection().bone.num;
	if (n == 0) {
		session->set_message(_("nothing selected"));
		return;
	} else if (n == 1) {
		temp.clear();
		ModelFrame f;
		foreachi(auto &b, data->bone, i)
			if (b.is_selected) {
				f.skel_ang.add(parent->cur_move()->frame[current_frame].skel_ang[i]);
				f.skel_dpos.add(parent->cur_move()->frame[current_frame].skel_dpos[i]);
			}
		temp.add(f);
		session->set_message(format(_("Animation copied from single bone"), n));
	} else {
		temp.clear();
		temp.add(parent->cur_move()->frame[current_frame]);
		session->set_message(format(_("copied animation of %d bones"), data->bone.num));
	}
}

void ModeModelAnimationSkeleton::paste() {
	if (temp.num == 0) {
		session->set_message(_("Clipboard is empty"));
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
			session->set_message(_("Animation pasted - single bone"));
		else
			session->set_message(_("Animation pasted - single bone onto multiple"));
	} else {
		foreachi(auto &b, data->bone, i)
			if (b.is_selected)
				data->animationSetBone(current_move, current_frame, i, temp[0].skel_dpos[i], temp[0].skel_ang[i]);
		session->set_message(format(_("Animation inserted - on %d bones"), n));
	}
	data->end_action_group();
}
