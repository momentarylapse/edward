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
	EmptyMove->Name = "-empty move-";
	EmptyMove->Type = MoveTypeNone;
	EmptyMove->Frame.resize(1);
	EmptyMove->FramesPerSecConst = 1;
	EmptyMove->FramesPerSecFactor = 0;
	EmptyMove->InterpolatedQuadratic = 0;
	EmptyMove->InterpolatedLoop = false;

	move = EmptyMove;
}


ModeModelAnimation::~ModeModelAnimation()
{
	delete(mode_model_animation_none);
	delete(mode_model_animation_skeleton);
	delete(mode_model_animation_vertex);
	delete(EmptyMove);
}



void ModeModelAnimation::OnCommand(const string & id)
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



void ModeModelAnimation::OnStart()
{
	TimeScale = 1;
	TimeParam = 0;
	Playing = false;
	PlayLoop = true;
	CurrentMove = -1;
	CurrentFrame = 0;

	dialog = new ModelAnimationDialog(ed, data);

	UpdateAnimation();
	Observer::Subscribe(data);
	OnUpdate(data);

	timer.reset();
	HuiRunLaterM(0.200f, this, &ModeModelAnimation::IdleFunction);

	ed->SetMode(mode_model_animation_none);
	Notify("Change");
}


void ModeModelAnimation::OnUpdateMenu()
{
}



void ModeModelAnimation::OnEnd()
{
	Observer::Unsubscribe(data);
	delete(dialog);
}



void ModeModelAnimation::SetCurrentMove(int move_no)
{
	move = EmptyMove;
	CurrentMove = -1;
	if ((move_no >= 0) && (move_no < data->Move.num))
		if (data->Move[move_no].Frame.num > 0){
			move = &data->Move[move_no];
			CurrentMove = move_no;
		}
	SetCurrentFrame(0);

	if (move->Type == MoveTypeSkeletal)
		ed->SetMode(mode_model_animation_skeleton);
	else if (move->Type == MoveTypeVertex)
		ed->SetMode(mode_model_animation_vertex);
	else
		ed->SetMode(mode_model_animation_none);
}

void ModeModelAnimation::SetCurrentFrame(int frame_no)
{
	if ((frame_no >= 0) && (frame_no < move->Frame.num)){
		CurrentFrame = frame_no;
		UpdateAnimation();
		Notify("Change");
	}
}

void ModeModelAnimation::SetCurrentFrameNext()
{
	SetCurrentFrame((CurrentFrame + 1) % move->Frame.num);
}

void ModeModelAnimation::SetCurrentFramePrevious()
{
	SetCurrentFrame((CurrentFrame - 1 + move->Frame.num) % move->Frame.num);
}

void ModeModelAnimation::UpdateAnimation()
{
	vertex.resize(data->Vertex.num);
	foreachi(ModelVertex &v, data->Vertex, i){
		vertex[i].view_stage = v.view_stage;
		vertex[i].is_selected = v.is_selected;
	}

	if (move->Type == MoveTypeSkeletal){
		UpdateSkeleton();
		foreachi(ModelVertex &v, data->Vertex, i){
			if (v.BoneIndex >= data->Bone.num)
				vertex[i].pos = v.pos;
			else
				vertex[i].pos = data->Bone[v.BoneIndex].Matrix * (v.pos - data->GetBonePos(v.BoneIndex));
		}
	}else if (move->Type == MoveTypeVertex){
		int frame0 = CurrentFrame;
		int frame1 = CurrentFrame;
		float t = 0;
		if (Playing){
			frame0 = SimFrame;
			frame1 = (frame0 + 1) % move->Frame.num;
			t = SimFrame - frame0;
		}
		foreachi(ModelVertex &v, data->Vertex, i){
			vertex[i].pos = v.pos + (1 - t) * move->Frame[frame0].VertexDPos[i] + t * move->Frame[frame1].VertexDPos[i];
		}
	}else{
		vertex = data->Vertex;
	}
	//data->Notify("Change");
	ed->ForceRedraw();
}

void ModeModelAnimation::UpdateSkeleton()
{
	if (move->Type != MoveTypeSkeletal){
		foreachi(ModelBone &b, data->Bone, i){
			if (b.Parent < 0)
				b.pos = b.DeltaPos;
			else
				b.pos = data->Bone[b.Parent].pos + b.DeltaPos;
		}
		return;
	}
	int frame0 = CurrentFrame;
	int frame1 = CurrentFrame;
	float t = 0;
	if (Playing){
		frame0 = SimFrame;
		frame1 = (frame0 + 1) % move->Frame.num;
		t = SimFrame - frame0;
	}

	foreachi(ModelBone &b, data->Bone, i){
		if (b.Parent < 0){
			b.pos = b.DeltaPos + (1 - t) * move->Frame[frame0].SkelDPos[i] + t * move->Frame[frame1].SkelDPos[i];
		}else{
			vector dp = data->Bone[b.Parent].Matrix.transform_normal(b.DeltaPos);
			b.pos = data->Bone[b.Parent].pos + dp;
		}
		matrix trans;
		MatrixTranslation(trans, b.pos);
		quaternion q0, q1, q;
		QuaternionRotationV(q0, move->Frame[frame0].SkelAng[i]);
		QuaternionRotationV(q1, move->Frame[frame1].SkelAng[i]);
		QuaternionInterpolate(q, q0, q1, t);
		MatrixRotationQ(b.RotMatrix, q);
		b.Matrix = trans * b.RotMatrix;
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
		SimFrame += dt * (move->FramesPerSecConst + move->FramesPerSecFactor * TimeParam) * TimeScale;
		if (SimFrame > move->Frame.num)
			SimFrame = 0;
		UpdateAnimation();
	}
}

void ModeModelAnimation::OnUpdate(Observable *o)
{
	msg_write("..up");
	UpdateAnimation();
	// valid move
	/*if (...data->Move[CurrentMove] == index)
		SetCurrentMove(-1);*/
}



void ModeModelAnimation::OnDraw()
{
	IterateAnimation(timer.get());
}

void ModeModelAnimation::OnDrawWin(MultiView::Window *win)
{
}

void ModeModelAnimation::IdleFunction()
{
	if (!IsAncestorOf(ed->cur_mode))
		return;
	ed->ForceRedraw();
	if (Playing)
		HuiRunLaterM(0.020f, this, &ModeModelAnimation::IdleFunction);
	else
		HuiRunLaterM(0.200f, this, &ModeModelAnimation::IdleFunction);
}


