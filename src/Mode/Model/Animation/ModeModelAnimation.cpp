/*
 * ModeModelAnimation.cpp
 *
 *  Created on: 14.03.2012
 *      Author: michi
 */

#include "../../../Edward.h"
#include "../../../MultiView/MultiView.h"
#include "ModeModelAnimation.h"
#include "ModeModelAnimationNone.h"
#include "ModeModelAnimationSkeleton.h"
#include "ModeModelAnimationVertex.h"
#include "Creation/ModeModelAnimationInterpolateFrames.h"
#include "../ModeModel.h"
#include "../Dialog/ModelAnimationDialog.h"
#include "../Dialog/ModelAnimationTimelinePanel.h"
#include "../Mesh/Selection/MeshSelectionModePolygon.h"

ModeModelAnimation *mode_model_animation = NULL;

const string ModeModelAnimation::MESSAGE_SET_FRAME = "SetFrame";

ModeModelAnimation::ModeModelAnimation(ModeBase *_parent) :
	Mode<DataModel>("ModelAnimation", _parent, ed->multi_view_3d, "menu_move"),
	Observable("ModelAnimation")
{
	mode_model_animation_none = new ModeModelAnimationNone(this);
	mode_model_animation_skeleton = new ModeModelAnimationSkeleton(this);
	mode_model_animation_vertex = new ModeModelAnimationVertex(this);

	// create one dummy animation
	empty_move = new ModelMove;
	empty_move->name = "-empty move-";
	empty_move->type = MOVE_TYPE_NONE;
	empty_move->frames_per_sec_const = 1;
	empty_move->frames_per_sec_factor = 0;
	empty_move->interpolated_quadratic = 0;
	empty_move->interpolated_loop = false;

	dialog = NULL;
	timeline = NULL;
	sim_frame_time = 0;
	play_loop = false;
	playing = false;
	time_param = 0;
	time_scale = 1;
	current_move = current_frame = 0;
}


ModeModelAnimation::~ModeModelAnimation()
{
	delete(mode_model_animation_none);
	delete(mode_model_animation_skeleton);
	delete(mode_model_animation_vertex);
	delete(empty_move);
}



void ModeModelAnimation::on_command(const string & id)
{
	if (id == "move_frame_inc")
		setCurrentFrameNext();
	if (id == "move_frame_dec")
		setCurrentFramePrevious();
	if (id == "move_frame_delete")
		deleteCurrentFrame();
	if (id == "move_frame_insert")
		duplicateCurrentFrame();
	if (id == "move_frame_interpolate")
		ed->set_mode(new ModeModelAnimationInterpolateFrames(ed->cur_mode));
}



void ModeModelAnimation::on_start()
{
	time_scale = 1;
	time_param = 0;
	playing = false;
	play_loop = true;
	current_move = -1;
	current_frame = 0;

	dialog = new ModelAnimationDialog(data);
	ed->embed(dialog, "root-table", 1, 0);
	timeline = new ModelAnimationTimelinePanel;

	ed->embed(timeline, "vgrid", 0, 1);

	Observer::subscribe(this, MESSAGE_SET_FRAME);
	Observer::subscribe(data);
	mode_model->allowSelectionModes(false);

	timer.reset();
	hui::RunLater(0.200f, std::bind(&ModeModelAnimation::idleFunction, this));
	setCurrentMove(getFirstMove());

	//ed->set_mode(mode_model_animation_none);
}

int ModeModelAnimation::getFirstMove()
{
	foreachi(ModelMove &m, data->move, i)
		if (m.frame.num > 0)
			return i;
	return -1;
}


void ModeModelAnimation::on_update_menu()
{
}



void ModeModelAnimation::on_end()
{
	Observer::unsubscribe(data);
	Observer::unsubscribe(this);
	delete(dialog);
	delete(timeline);
}



void ModeModelAnimation::setCurrentMove(int move_no)
{
	current_move = -1;
	if ((move_no >= 0) && (move_no < data->move.num))
		if (data->move[move_no].frame.num > 0){
			current_move = move_no;
		}
	setCurrentFrame(0);

	if (cur_move()->type == MOVE_TYPE_SKELETAL)
		ed->set_mode(mode_model_animation_skeleton);
	else if (cur_move()->type == MOVE_TYPE_VERTEX)
		ed->set_mode(mode_model_animation_vertex);
	else
		ed->set_mode(mode_model_animation_none);
}

void ModeModelAnimation::setCurrentFrame(int frame_no)
{
	current_frame = loopi(frame_no, 0, cur_move()->frame.num - 1);
	//updateAnimation();
	notify(MESSAGE_SET_FRAME);
}

void ModeModelAnimation::setCurrentFrameNext()
{
	setCurrentFrame(current_frame + 1);
}

void ModeModelAnimation::setCurrentFramePrevious()
{
	setCurrentFrame(current_frame - 1);
}

void ModeModelAnimation::updateAnimation()
{
	vertex.resize(data->vertex.num);
	foreachi(ModelVertex &v, data->vertex, i){
		vertex[i].view_stage = v.view_stage;
		vertex[i].is_selected = v.is_selected;
	}

	if (cur_move()->type == MOVE_TYPE_SKELETAL){
		updateSkeleton();
		foreachi(ModelVertex &v, data->vertex, i){
			if (v.bone_index >= data->bone.num){
				vertex[i].pos = v.pos;
			}else{
				ModelBone &b = data->bone[v.bone_index];
				vertex[i].pos = b._matrix * (v.pos - b.pos);
			}
		}
	}else if (cur_move()->type == MOVE_TYPE_VERTEX){
		ModelFrame f = getInterpolation();
		foreachi(ModelVertex &v, data->vertex, i)
			vertex[i].pos = v.pos + f.vertex_dpos[i];
	}else{
		vertex = data->vertex;
	}


	mode_model_mesh->updateVertexBuffers(vertex);
	mode_model_mesh->fillSelectionBuffer(vertex);

	notify();
	ed->force_redraw();
}

void ModeModelAnimation::updateSkeleton()
{
	bone = data->bone;

	if (cur_move()->type != MOVE_TYPE_SKELETAL){
		return;
	}
	ModelFrame f = getInterpolation();

	foreachi(ModelBone &b, data->bone, i){
		if (b.parent < 0){
			bone[i].pos = b.pos + f.skel_dpos[i];
		}else{
			ModelBone &pb = data->bone[b.parent];
			bone[i].pos = pb._matrix * (b.pos - pb.pos); // cur_mat * dpos_at_rest
		}
		matrix trans, rot;
		trans = matrix::translation( bone[i].pos);
		rot = matrix::rotation(f.skel_ang[i]);
		b._matrix = trans * rot;
		bone[i]._matrix = b._matrix;
	}
}

ModelFrame ModeModelAnimation::getInterpolation()
{
	if (playing)
		return cur_move()->interpolate(sim_frame_time);
	return cur_move()->frame[current_frame];
}

void ModeModelAnimation::deleteCurrentFrame()
{
	if (cur_move()->frame.num > 1)
		data->animationDeleteFrame(current_move, current_frame);
	else
		ed->set_message(_("der letzte Frame kann nicht gel&oscht werden"));
}

void ModeModelAnimation::duplicateCurrentFrame()
{
	if (current_frame > 0)
		data->animationAddFrame(current_move, current_frame + 1, cur_move()->frame[current_frame]);
	else
		data->animationAddFrame(current_move, current_frame + 1, cur_move()->frame[0]);
	setCurrentFrame(current_frame + 1);
}

void ModeModelAnimation::iterateAnimation(float dt)
{
	if (playing){
		sim_frame_time += dt * (cur_move()->frames_per_sec_const + cur_move()->frames_per_sec_factor * time_param) * time_scale;
		sim_frame_time = loopf(sim_frame_time, 0, cur_move()->duration());
		updateAnimation();
	}
}

ModelMove* ModeModelAnimation::cur_move()
{
	if ((current_move >= 0) and (current_move < data->move.num))
		return &data->move[current_move];
	return empty_move;
}

void ModeModelAnimation::on_update(Observable *o, const string &message)
{
	// consistency check
	if (((current_move >= 0) and (cur_move()->frame.num == 0)) or (current_move >= data->move.num))
		setCurrentMove(getFirstMove());
	else if ((current_move < 0) and (getFirstMove() >= 0))
		setCurrentMove(getFirstMove());

	if ((current_frame >= cur_move()->frame.num) and (cur_move()->frame.num > 0))
		setCurrentFrame(cur_move()->frame.num - 1);
	else if (current_frame < 0)
		setCurrentFrame(0);

	updateAnimation();
}


void ModeModelAnimation::idleFunction()
{
	if (!is_ancestor_of(ed->cur_mode))
		return;

	iterateAnimation(timer.get());

	if (playing)
		hui::RunLater(0.020f, std::bind(&ModeModelAnimation::idleFunction, this));
	else
		hui::RunLater(0.200f, std::bind(&ModeModelAnimation::idleFunction, this));
}


