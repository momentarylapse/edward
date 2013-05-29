/*
 * ModelAnimationDialog.cpp
 *
 *  Created on: 13.08.2012
 *      Author: michi
 */

#include "ModelAnimationDialog.h"
#include "../../../Data/Model/DataModel.h"
#include "ModelNewAnimationDialog.h"
#include "../Animation/ModeModelAnimation.h"

static DataModel *__data = NULL;
static int timer = -1;

void _later_func_()
{
	if (!__data)
		return;
	float dt = HuiGetTime(timer);
	mode_model_animation->IterateAnimation(dt);
	if (mode_model_animation->Playing){
		mode_model_animation->SimFrame += dt * (mode_model_animation->move->FramesPerSecConst + mode_model_animation->move->FramesPerSecFactor * mode_model_animation->TimeParam) * mode_model_animation->TimeScale;
		if (mode_model_animation->SimFrame > mode_model_animation->move->Frame.num)
			mode_model_animation->SimFrame = 0;
		mode_model_animation->UpdateAnimation();
		HuiRunLater(20, &_later_func_);
	}else
		HuiRunLater(200, &_later_func_);
}

ModelAnimationDialog::ModelAnimationDialog(CHuiWindow *_parent, bool _allow_parent, DataModel *_data) :
	CHuiWindow("dummy", -1, -1, 230, 400, _parent, _allow_parent, HuiWinModeControls, true)
{
	data = _data;
	__data = data;
	if (timer < 0)
		timer = HuiCreateTimer();

	// dialog
	FromResource("animation_dialog");
	SetPositionSpecial(_parent, HuiRight | HuiTop);

	EventM("hui:close", this, &ModelAnimationDialog::OnClose);
	EventM("animation_list", this, &ModelAnimationDialog::OnAnimationList);
	EventMX("animation_list", "hui:select", this, &ModelAnimationDialog::OnAnimationListSelect);
	EventM("animation_new", this, &ModelAnimationDialog::OnAddAnimation);
	EventM("animation_delete", this, &ModelAnimationDialog::OnDeleteAnimation);
	EventM("frame_inc", this, &ModelAnimationDialog::OnFrameInc);
	EventM("frame_dec", this, &ModelAnimationDialog::OnFrameDec);
	EventM("frame", this, &ModelAnimationDialog::OnFrame);
	EventM("new_frame", this, &ModelAnimationDialog::OnAddFrame);
	EventM("delete_frame", this, &ModelAnimationDialog::OnDeleteFrame);
	EventM("name", this, &ModelAnimationDialog::OnName);
	EventM("fps_const", this, &ModelAnimationDialog::OnFpsConst);
	EventM("fps_factor", this, &ModelAnimationDialog::OnFpsFactor);
	EventM("speed", this, &ModelAnimationDialog::OnSpeed);
	EventM("parameter", this, &ModelAnimationDialog::OnParameter);
	EventM("animation_dialog_tab_control", this, &ModelAnimationDialog::OnTabControl);

	Subscribe(data);

	LoadData();
	HuiRunLater(200, &_later_func_);
}

ModelAnimationDialog::~ModelAnimationDialog()
{
	Unsubscribe(data);
	__data = NULL;
}

void ModelAnimationDialog::LoadData()
{
	Reset("animation_list");
	int n = 0;
	foreachi(ModelMove &m, data->Move, i)
		if (m.Frame.num > 0){
			string str = i2s(i) + "\\";
			if (m.Type == MoveTypeVertex)
				str += _("Vertex");
			else if (m.Type == MoveTypeSkeletal)
				str += _("Skelett");
			else
				str += "???";
			str += format("\\%d\\", m.Frame.num) + m.Name;
			AddString("animation_list", str);
			if (i == mode_model_animation->CurrentMove)
				SetInt("animation_list", n);
			n ++;
		}
	FillAnimation();
	SetDecimals(1);
	SetFloat("speed", mode_model_animation->TimeScale * 100.0f);
	SetDecimals(3);
	SetFloat("parameter", mode_model_animation->TimeParam);
}

void ModelAnimationDialog::FillAnimation()
{
	bool b = false;
	if ((mode_model_animation->CurrentMove >= 0) && (mode_model_animation->CurrentMove < data->Move.num)){
		mode_model_animation->move = &data->Move[mode_model_animation->CurrentMove];
		b = mode_model_animation->move->Frame.num > 0;
	}
	Enable("name", b);
	Enable("frame", b);
	Enable("frame_inc", b);
	Enable("frame_dec", b);
	Enable("new_frame", b);
	Enable("delete_frame", b);
	Enable("num_frames_set", b);
	Enable("num_frames_wanted", b);
	Enable("fps_const", b);
	Enable("fps_factor", b);
	Enable("interpolate_quad", b);
	Enable("interpolate_loop", b);
	if (b){
		ModelMove *move = mode_model_animation->move;
		SetString("name", move->Name);
		SetInt("frame", mode_model_animation->CurrentFrame);
		SetInt("num_frames_wanted", move->Frame.num);
		SetFloat("fps_const", move->FramesPerSecConst);
		SetFloat("fps_factor", move->FramesPerSecFactor);
		Enable("interpolate_quad", move->Type==MoveTypeSkeletal);
		Check("interpolate_quad", move->InterpolatedQuadratic && move->Type==MoveTypeSkeletal);
		Enable("interpolate_loop", move->InterpolatedQuadratic && move->Type==MoveTypeSkeletal);
		Check("interpolate_loop", move->InterpolatedLoop && move->Type==MoveTypeSkeletal);
	}
}

int ModelAnimationDialog::GetSelectedAnimation()
{
	int s = GetInt("animation_list");
	if (s >= 0){
		int n = 0;
		foreachi(ModelMove &m, data->Move, i)
			if (m.Frame.num > 0){
				if (n == s)
					return i;
				n ++;
			}
	}
	return -1;
}

void ModelAnimationDialog::OnAnimationList()
{
	int s = GetSelectedAnimation();
	mode_model_animation->SetCurrentMove(s);
	if (s >= 0)
		SetInt("animation_dialog_tab_control", 1);
}

void ModelAnimationDialog::OnAnimationListSelect()
{
	int s = GetSelectedAnimation();
	mode_model_animation->SetCurrentMove(s);
}

void ModelAnimationDialog::OnClose()
{
}

void ModelAnimationDialog::ApplyData()
{
}

void ModelAnimationDialog::OnAddAnimation()
{
	ModelNewAnimationDialog *dlg = new ModelNewAnimationDialog(this, false, data, 0);
	dlg->Update();
	HuiWaitTillWindowClosed(dlg);
}

void ModelAnimationDialog::OnDeleteAnimation()
{
	int s = GetSelectedAnimation();
	if (s >= 0)
		data->DeleteAnimation(s);
}

void ModelAnimationDialog::OnFrameInc()
{
	mode_model_animation->SetCurrentFrameNext();
}

void ModelAnimationDialog::OnFrameDec()
{
	mode_model_animation->SetCurrentFramePrevious();
}

void ModelAnimationDialog::OnFrame()
{
	mode_model_animation->SetCurrentFrame(GetInt(""));
}

void ModelAnimationDialog::OnAddFrame()
{
	mode_model_animation->AnimationDuplicateCurrentFrame();
}

void ModelAnimationDialog::OnDeleteFrame()
{
	mode_model_animation->AnimationDeleteCurrentFrame();
}

void ModelAnimationDialog::OnName()
{
	mode_model_animation->move->Name = GetString("");
}

void ModelAnimationDialog::OnFpsConst()
{
	mode_model_animation->move->FramesPerSecConst = GetFloat("");
}

void ModelAnimationDialog::OnFpsFactor()
{
	mode_model_animation->move->FramesPerSecFactor = GetFloat("");
}

void ModelAnimationDialog::OnSpeed()
{
	mode_model_animation->TimeScale = GetFloat("") / 100.0f;
}

void ModelAnimationDialog::OnParameter()
{
	mode_model_animation->TimeParam = GetFloat("");
}

void ModelAnimationDialog::OnTabControl()
{
	int s = GetInt("");
	mode_model_animation->Playing = (s == 2) && (mode_model_animation->move->Frame.num > 0);
	mode_model_animation->SimFrame = 0;
	mode_model_animation->UpdateAnimation();
}

void ModelAnimationDialog::OnUpdate(Observable *o)
{
	LoadData();
}

