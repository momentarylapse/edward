/*
 * ModeModelAnimation.cpp
 *
 *  Created on: 14.03.2012
 *      Author: michi
 */

#include "ModeModelAnimation.h"
#include "ModeModelAnimationNone.h"
#include "ModeModelAnimationSkeleton.h"
#include "ModeModelAnimationVertex.h"
#include "creation/ModeModelAnimationInterpolateFrames.h"
#include "../ModeModel.h"
#include "../dialog/ModelAnimationDialog.h"
#include "../dialog/ModelAnimationTimelinePanel.h"
#include "../mesh/selection/MeshSelectionModePolygon.h"
#include "../../../Session.h"
#include "../../../EdwardWindow.h"
#include "../../../multiview/MultiView.h"
#include "../../../data/model/DataModel.h"
#include "../../../data/model/ModelMesh.h"
#include <y/world/components/Animator.h>

const string ModeModelAnimation::State::MESSAGE_SET_FRAME = "set-frame";

ModeModelAnimation::ModeModelAnimation(ModeModel *_parent, MultiView::MultiView *mv) :
	Mode<ModeModel, DataModel>(_parent->session, "ModelAnimation", _parent, mv, "menu_move"),
	in_update(this, [this] { on_update(); })
{
	mode_model_animation_none = new ModeModelAnimationNone(this, mv);
	mode_model_animation_skeleton = new ModeModelAnimationSkeleton(this, mv);
	mode_model_animation_vertex = new ModeModelAnimationVertex(this, mv);

	// create one dummy animation
	empty_move = new ModelMove;
	empty_move->name = "-empty move-";
	empty_move->type = AnimationType::NONE;
	empty_move->frames_per_sec_const = 1;
	empty_move->frames_per_sec_factor = 0;
	empty_move->interpolated_quadratic = 0;
	empty_move->interpolated_loop = false;

	dialog = nullptr;
	timeline = nullptr;
	runner = -1;
	sim_frame_time = 0;
	play_loop = false;
	playing = false;
	time_param = 0;
	time_scale = 1;
	current_move = current_frame = 0;
}


ModeModelAnimation::~ModeModelAnimation() {
	delete mode_model_animation_none;
	delete mode_model_animation_skeleton;
	delete mode_model_animation_vertex;
	delete empty_move;
}



void ModeModelAnimation::on_command(const string & id) {
	if (id == "move_frame_inc")
		set_current_frame_next();
	if (id == "move_frame_dec")
		set_current_frame_previous();
	if (id == "move_frame_delete")
		delete_current_frame();
	if (id == "move_frame_insert")
		duplicate_current_frame();
	if (id == "move_frame_interpolate")
		session->set_mode(new ModeModelAnimationInterpolateFrames(this));
}



void ModeModelAnimation::on_start() {
	time_scale = 1;
	time_param = 0;
	playing = false;
	play_loop = true;
	current_move = -1;
	current_frame = 0;

	dialog = new ModelAnimationDialog(data);
	session->win->set_side_panel(dialog);

	timeline = new ModelAnimationTimelinePanel(data);
	session->win->set_bottom_panel(timeline);


	data->out_changed >> in_update;
	data->out_selection >> in_update;
	session->mode_model->allow_selection_modes(false);

	timer.reset();
	runner = hui::run_repeated(0.020f, [this] { idle_function(); });
	set_current_move(getFirstMove());

	//session->set_mode(mode_model_animation_none);
}

int ModeModelAnimation::getFirstMove() {
	foreachi(ModelMove &m, data->move, i)
		if (m.frame.num > 0)
			return i;
	return -1;
}


void ModeModelAnimation::on_update_menu() {
}



void ModeModelAnimation::on_end() {
	hui::cancel_runner(runner);
	data->unsubscribe(this);
	session->win->set_side_panel(nullptr);
	session->win->set_bottom_panel(nullptr);
}



void ModeModelAnimation::set_current_move(int move_no) {
	current_move = -1;
	if ((move_no >= 0) && (move_no < data->move.num))
		if (data->move[move_no].frame.num > 0) {
			current_move = move_no;
		}
	set_current_frame(0);

	if (cur_move()->type == AnimationType::SKELETAL)
		session->set_mode(mode_model_animation_skeleton);
	else if (cur_move()->type == AnimationType::VERTEX)
		session->set_mode(mode_model_animation_vertex);
	else
		session->set_mode(mode_model_animation_none);
}

void ModeModelAnimation::set_current_frame(int frame_no) {
	current_frame = loop(frame_no, 0, cur_move()->frame.num);
	//updateAnimation();
	state.out_set_frame.notify();
	on_update();
}

void ModeModelAnimation::set_current_frame_next() {
	set_current_frame(current_frame + 1);
}

void ModeModelAnimation::set_current_frame_previous() {
	set_current_frame(current_frame - 1);
}

void ModeModelAnimation::update_animation() {
	vertex.resize(data->mesh->vertex.num);
	foreachi(auto &v, data->mesh->vertex, i) {
		vertex[i].view_stage = v.view_stage;
		vertex[i].is_selected = v.is_selected;
	}

	if (cur_move()->type == AnimationType::SKELETAL) {
		update_skeleton();

		foreachi(auto &v, data->mesh->vertex, i){
			vec3 pp = v_0;
			for (int k=0; k<4; k++) {
				if ((&v.bone_index.i)[k] < 0)
					continue;

				ModelBone &b = data->bone[(&v.bone_index.i)[k]];
				pp += (b._matrix * (v.pos - b.pos)) * (&v.bone_weight.x)[k];
			}
			vertex[i].pos = pp;
		}
	} else if (cur_move()->type == AnimationType::VERTEX) {
		auto f = get_interpolation();
		foreachi(auto &v, data->mesh->vertex, i)
			vertex[i].pos = v.pos + f.vertex_dpos[i];
	} else {
		vertex = data->mesh->vertex;
	}


	session->mode_model->mode_model_mesh->update_vertex_buffers(vertex);
	session->mode_model->mode_model_mesh->fill_selection_buffer(vertex);

	state.out_changed.notify();
	multi_view->force_redraw();
}

void ModeModelAnimation::update_skeleton() {
	bone = data->bone;

	if (cur_move()->type != AnimationType::SKELETAL) {
		return;
	}
	auto f = get_interpolation();

	foreachi(auto &b, data->bone, i) {
		if (b.parent < 0) {
			bone[i].pos = b.pos + f.skel_dpos[i];
		} else {
			ModelBone &pb = data->bone[b.parent];
			bone[i].pos = pb._matrix * (b.pos - pb.pos); // cur_mat * dpos_at_rest
		}
		auto trans = mat4::translation( bone[i].pos);
		auto rot = mat4::rotation(f.skel_ang[i]);
		b._matrix = trans * rot;
		bone[i]._matrix = b._matrix;
	}
}

ModelFrame ModeModelAnimation::get_interpolation() {
	if (playing)
		return cur_move()->interpolate(sim_frame_time);
	return cur_move()->frame[current_frame];
}

void ModeModelAnimation::delete_current_frame() {
	if (cur_move()->frame.num > 1)
		data->animationDeleteFrame(current_move, current_frame);
	else
		session->set_message(_("can not delete the only frame"));
}

void ModeModelAnimation::duplicate_current_frame() {
	if (current_frame > 0)
		data->animationAddFrame(current_move, current_frame + 1, cur_move()->frame[current_frame]);
	else
		data->animationAddFrame(current_move, current_frame + 1, cur_move()->frame[0]);
	set_current_frame(current_frame + 1);
}

void ModeModelAnimation::iterate_animation(float dt) {
	if (playing) {
		sim_frame_time += dt * (cur_move()->frames_per_sec_const + cur_move()->frames_per_sec_factor * time_param) * time_scale;
		sim_frame_time = loop(sim_frame_time, 0.0f, cur_move()->duration());
		update_animation();
	}
}

ModelMove* ModeModelAnimation::cur_move() {
	if ((current_move >= 0) and (current_move < data->move.num))
		return &data->move[current_move];
	return empty_move;
}

void ModeModelAnimation::on_update() {
	// consistency check
	if (((current_move >= 0) and (cur_move()->frame.num == 0)) or (current_move >= data->move.num))
		set_current_move(getFirstMove());
	else if ((current_move < 0) and (getFirstMove() >= 0))
		set_current_move(getFirstMove());

	if ((current_frame >= cur_move()->frame.num) and (cur_move()->frame.num > 0))
		set_current_frame(cur_move()->frame.num - 1);
	else if (current_frame < 0)
		set_current_frame(0);

	update_animation();
}


void ModeModelAnimation::idle_function() {
	if (!is_ancestor_of(session->cur_mode))
		return;

	// update slow, when not playing
	if (!playing)
		if (timer.peek() < 0.2f)
			return;

	iterate_animation(timer.get());
}


