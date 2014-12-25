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
#include "../ModeModel.h"
#include "../Dialog/ModelAnimationDialog.h"
#include "../Dialog/ModelAnimationTimelinePanel.h"
#include "../Mesh/MeshSelectionModePolygon.h"

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
}


ModeModelAnimation::~ModeModelAnimation()
{
	delete(mode_model_animation_none);
	delete(mode_model_animation_skeleton);
	delete(mode_model_animation_vertex);
	delete(empty_move);
}



void ModeModelAnimation::onCommand(const string & id)
{
	if (id == "move_frame_inc")
		setCurrentFrameNext();
	if (id == "move_frame_dec")
		setCurrentFramePrevious();
	if (id == "move_frame_delete")
		deleteCurrentFrame();
	if (id == "move_frame_insert")
		duplicateCurrentFrame();
}



void ModeModelAnimation::onStart()
{
	time_scale = 1;
	time_param = 0;
	playing = false;
	play_loop = true;
	current_move = -1;
	current_frame = 0;

	dialog = new ModelAnimationDialog(ed, data);
	timeline = new ModelAnimationTimelinePanel;

	ed->embed(timeline, "vgrid", 0, 1);

	Observer::subscribe(this, MESSAGE_SET_FRAME);
	Observer::subscribe(data);
	mode_model->allowSelectionModes(false);

	timer.reset();
	HuiRunLaterM(0.200f, this, &ModeModelAnimation::idleFunction);
	setCurrentMove(getFirstMove());

	//ed->setMode(mode_model_animation_none);
}

int ModeModelAnimation::getFirstMove()
{
	foreachi(ModelMove &m, data->move, i)
		if (m.frame.num > 0)
			return i;
	return -1;
}


void ModeModelAnimation::onUpdateMenu()
{
}



void ModeModelAnimation::onEnd()
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
		ed->setMode(mode_model_animation_skeleton);
	else if (cur_move()->type == MOVE_TYPE_VERTEX)
		ed->setMode(mode_model_animation_vertex);
	else
		ed->setMode(mode_model_animation_none);
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
		int frame0, frame1;
		float t;
		getTimeInterpolation(frame0, frame1, t);
		foreachi(ModelVertex &v, data->vertex, i){
			vertex[i].pos = v.pos + (1 - t) * cur_move()->frame[frame0].vertex_dpos[i] + t * cur_move()->frame[frame1].vertex_dpos[i];
		}
	}else{
		vertex = data->vertex;
	}


	mode_model_mesh->updateVertexBuffers(vertex);
	mode_model_mesh->fillSelectionBuffer(vertex);

	notify();
	ed->forceRedraw();
}

void ModeModelAnimation::updateSkeleton()
{
	bone = data->bone;

	if (cur_move()->type != MOVE_TYPE_SKELETAL){
		return;
	}
	int frame0, frame1;
	float t;
	getTimeInterpolation(frame0, frame1, t);

	foreachi(ModelBone &b, data->bone, i){
		if (b.parent < 0){
			bone[i].pos = b.pos + (1 - t) * cur_move()->frame[frame0].skel_dpos[i] + t * cur_move()->frame[frame1].skel_dpos[i];
		}else{
			ModelBone &pb = data->bone[b.parent];
			bone[i].pos = pb._matrix * (b.pos - pb.pos); // cur_mat * dpos_at_rest
		}
		matrix trans, rot;
		MatrixTranslation(trans, bone[i].pos);
		quaternion q0, q1, q;
		QuaternionRotationV(q0, cur_move()->frame[frame0].skel_ang[i]);
		QuaternionRotationV(q1, cur_move()->frame[frame1].skel_ang[i]);
		QuaternionInterpolate(q, q0, q1, t);
		MatrixRotationQ(rot, q);
		b._matrix = trans * rot;
		bone[i]._matrix = b._matrix;
	}
}

void ModeModelAnimation::getTimeInterpolation(int &frame0, int &frame1, float &t)
{
	frame0 = current_frame;
	frame1 = current_frame;
	t = 0;

	if (playing){
		float t0 = 0;
		foreachi(ModelFrame &f, cur_move()->frame, i){
			if (sim_frame_time < t0 + f.duration){
				frame0 = i;
				frame1 = (i + 1) % cur_move()->frame.num;
				t = (sim_frame_time - t0) / f.duration;
				break;
			}
			t0 += f.duration;
		}
	}
}

void ModeModelAnimation::deleteCurrentFrame()
{
	if (cur_move()->frame.num > 1)
		data->animationDeleteFrame(current_move, current_frame);
	else
		ed->setMessage(_("der letzte Frame kann nicht gel&oscht werden"));
}

void ModeModelAnimation::duplicateCurrentFrame()
{
	data->animationAddFrame(current_move, current_frame + 1);
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

void ModeModelAnimation::onUpdate(Observable *o, const string &message)
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
	if (!isAncestorOf(ed->cur_mode))
		return;

	iterateAnimation(timer.get());

	if (playing)
		HuiRunLaterM(0.020f, this, &ModeModelAnimation::idleFunction);
	else
		HuiRunLaterM(0.200f, this, &ModeModelAnimation::idleFunction);
}


