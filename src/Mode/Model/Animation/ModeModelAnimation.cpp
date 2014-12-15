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
#include "../Mesh/ModeModelMeshPolygon.h"
#include "../Dialog/ModelAnimationDialog.h"

ModeModelAnimation *mode_model_animation = NULL;

ModeModelAnimation::ModeModelAnimation(ModeBase *_parent) :
	Mode<DataModel>("ModelAnimation", _parent, ed->multi_view_3d, "menu_move"),
	Observable("ModelAnimation")
{
	mode_model_animation_none = new ModeModelAnimationNone(this);
	mode_model_animation_skeleton = new ModeModelAnimationSkeleton(this);
	mode_model_animation_vertex = new ModeModelAnimationVertex(this);

	// create one dummy animation
	EmptyMove = new ModelMove;
	EmptyMove->name = "-empty move-";
	EmptyMove->type = MoveTypeNone;
	EmptyMove->frame.resize(1);
	EmptyMove->frames_per_sec_const = 1;
	EmptyMove->frames_per_sec_factor = 0;
	EmptyMove->interpolated_quadratic = 0;
	EmptyMove->interpolated_loop = false;

	move = EmptyMove;
}


ModeModelAnimation::~ModeModelAnimation()
{
	delete(mode_model_animation_none);
	delete(mode_model_animation_skeleton);
	delete(mode_model_animation_vertex);
	delete(EmptyMove);
}



void ModeModelAnimation::onCommand(const string & id)
{
	if (id == "move_frame_inc")
		SetCurrentFrameNext();
	if (id == "move_frame_dec")
		SetCurrentFramePrevious();
	if (id == "move_frame_delete")
		AnimationDeleteCurrentFrame();
	if (id == "move_frame_insert")
		AnimationDuplicateCurrentFrame();
}



void ModeModelAnimation::onStart()
{
	TimeScale = 1;
	TimeParam = 0;
	Playing = false;
	PlayLoop = true;
	CurrentMove = -1;
	CurrentFrame = 0;

	dialog = new ModelAnimationDialog(ed, data);

	UpdateAnimation();
	Observer::subscribe(data);
	onUpdate(data, "");

	timer.reset();
	HuiRunLaterM(0.200f, this, &ModeModelAnimation::IdleFunction);

	ed->setMode(mode_model_animation_none);
	notify();
}


void ModeModelAnimation::onUpdateMenu()
{
}



void ModeModelAnimation::onEnd()
{
	Observer::unsubscribe(data);
	delete(dialog);
}



void ModeModelAnimation::SetCurrentMove(int move_no)
{
	move = EmptyMove;
	CurrentMove = -1;
	if ((move_no >= 0) && (move_no < data->move.num))
		if (data->move[move_no].frame.num > 0){
			move = &data->move[move_no];
			CurrentMove = move_no;
		}
	SetCurrentFrame(0);

	if (move->type == MoveTypeSkeletal)
		ed->setMode(mode_model_animation_skeleton);
	else if (move->type == MoveTypeVertex)
		ed->setMode(mode_model_animation_vertex);
	else
		ed->setMode(mode_model_animation_none);
}

void ModeModelAnimation::SetCurrentFrame(int frame_no)
{
	if ((frame_no >= 0) && (frame_no < move->frame.num)){
		CurrentFrame = frame_no;
		UpdateAnimation();
		notify();
	}
}

void ModeModelAnimation::SetCurrentFrameNext()
{
	SetCurrentFrame((CurrentFrame + 1) % move->frame.num);
}

void ModeModelAnimation::SetCurrentFramePrevious()
{
	SetCurrentFrame((CurrentFrame - 1 + move->frame.num) % move->frame.num);
}

void ModeModelAnimation::UpdateAnimation()
{
	vertex.resize(data->vertex.num);
	foreachi(ModelVertex &v, data->vertex, i){
		vertex[i].view_stage = v.view_stage;
		vertex[i].is_selected = v.is_selected;
	}

	if (move->type == MoveTypeSkeletal){
		UpdateSkeleton();
		foreachi(ModelVertex &v, data->vertex, i){
			if (v.bone_index >= data->bone.num){
				vertex[i].pos = v.pos;
			}else{
				ModelBone &b = data->bone[v.bone_index];
				vertex[i].pos = b._matrix * (v.pos - b.pos);
			}
		}
	}else if (move->type == MoveTypeVertex){
		int frame0 = CurrentFrame;
		int frame1 = CurrentFrame;
		float t = 0;
		if (Playing){
			frame0 = SimFrame;
			frame1 = (frame0 + 1) % move->frame.num;
			t = SimFrame - frame0;
		}
		foreachi(ModelVertex &v, data->vertex, i){
			vertex[i].pos = v.pos + (1 - t) * move->frame[frame0].vertex_dpos[i] + t * move->frame[frame1].vertex_dpos[i];
		}
	}else{
		vertex = data->vertex;
	}

	mode_model_mesh_polygon->FillSelectionBuffers(vertex);

	//data->notify();
	ed->forceRedraw();
}

void ModeModelAnimation::UpdateSkeleton()
{
	bone = data->bone;

	if (move->type != MoveTypeSkeletal){
		return;
	}
	int frame0 = CurrentFrame;
	int frame1 = CurrentFrame;
	float t = 0;
	if (Playing){
		frame0 = SimFrame;
		frame1 = (frame0 + 1) % move->frame.num;
		t = SimFrame - frame0;
	}

	foreachi(ModelBone &b, data->bone, i){
		if (b.parent < 0){
			bone[i].pos = b.pos + (1 - t) * move->frame[frame0].skel_dpos[i] + t * move->frame[frame1].skel_dpos[i];
		}else{
			ModelBone &pb = data->bone[b.parent];
			bone[i].pos = pb._matrix * (b.pos - pb.pos); // cur_mat * dpos_at_rest
		}
		matrix trans, rot;
		MatrixTranslation(trans, bone[i].pos);
		quaternion q0, q1, q;
		QuaternionRotationV(q0, move->frame[frame0].skel_ang[i]);
		QuaternionRotationV(q1, move->frame[frame1].skel_ang[i]);
		QuaternionInterpolate(q, q0, q1, t);
		MatrixRotationQ(rot, q);
		b._matrix = trans * rot;
		bone[i]._matrix = b._matrix;
	}
}

void ModeModelAnimation::AnimationDeleteCurrentFrame()
{
	data->AnimationDeleteFrame(CurrentMove, CurrentFrame);
	SetCurrentMove(-1);
}

void ModeModelAnimation::AnimationDuplicateCurrentFrame()
{
	data->AnimationAddFrame(CurrentMove, CurrentFrame + 1);
	SetCurrentFrame(CurrentFrame + 1);
}

void ModeModelAnimation::IterateAnimation(float dt)
{
	if (Playing){
		SimFrame += dt * (move->frames_per_sec_const + move->frames_per_sec_factor * TimeParam) * TimeScale;
		SimFrame = loopf(SimFrame, 0, move->frame.num);
		UpdateAnimation();
	}
}

void ModeModelAnimation::onUpdate(Observable *o, const string &message)
{
	//msg_write("..up");
	UpdateAnimation();
	// valid move
	/*if (...data->Move[CurrentMove] == index)
		SetCurrentMove(-1);*/
}



void ModeModelAnimation::onDraw()
{
	IterateAnimation(timer.get());
}

void ModeModelAnimation::onDrawWin(MultiView::Window *win)
{
}

void ModeModelAnimation::IdleFunction()
{
	if (!isAncestorOf(ed->cur_mode))
		return;
	ed->forceRedraw();
	if (Playing)
		HuiRunLaterM(0.020f, this, &ModeModelAnimation::IdleFunction);
	else
		HuiRunLaterM(0.200f, this, &ModeModelAnimation::IdleFunction);
}


