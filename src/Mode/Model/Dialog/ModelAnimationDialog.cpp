/*
 * ModelAnimationDialog.cpp
 *
 *  Created on: 13.08.2012
 *      Author: michi
 */

#include "ModelAnimationDialog.h"
#include "../../../Data/Model/DataModel.h"
#include "ModelNewAnimationDialog.h"

static DataModel *__data = NULL;
static int timer = -1;

void _later_func_()
{
	if (!__data)
		return;
	float dt = HuiGetTime(timer);
	if (__data->Playing){
		__data->SimFrame += dt * (__data->move->FramesPerSecConst + __data->move->FramesPerSecFactor * __data->TimeParam) * __data->TimeScale;
		if (__data->SimFrame > __data->move->Frame.num)
			__data->SimFrame = 0;
		__data->UpdateAnimation();
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
			if (i == data->CurrentMove)
				SetInt("animation_list", n);
			n ++;
		}
	FillAnimation();
	SetDecimals(1);
	SetFloat("speed", data->TimeScale * 100.0f);
	SetDecimals(3);
	SetFloat("parameter", data->TimeParam);
}

void ModelAnimationDialog::FillAnimation()
{
	bool b = false;
	if ((data->CurrentMove >= 0) && (data->CurrentMove < data->Move.num)){
		data->move = &data->Move[data->CurrentMove];
		b = data->move->Frame.num > 0;
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
		SetString("name", data->move->Name);
		SetInt("frame", data->CurrentFrame);
		SetInt("num_frames_wanted", data->move->Frame.num);
		SetFloat("fps_const", data->move->FramesPerSecConst);
		SetFloat("fps_factor", data->move->FramesPerSecFactor);
		Enable("interpolate_quad", data->move->Type==MoveTypeSkeletal);
		Check("interpolate_quad", data->move->InterpolatedQuadratic && data->move->Type==MoveTypeSkeletal);
		Enable("interpolate_loop", data->move->InterpolatedQuadratic && data->move->Type==MoveTypeSkeletal);
		Check("interpolate_loop", data->move->InterpolatedLoop && data->move->Type==MoveTypeSkeletal);
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
	data->SetCurrentMove(s);
	if (s >= 0)
		SetInt("animation_dialog_tab_control", 1);
}

void ModelAnimationDialog::OnAnimationListSelect()
{
	int s = GetSelectedAnimation();
	data->SetCurrentMove(s);
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
	data->SetCurrentFrameNext();
}

void ModelAnimationDialog::OnFrameDec()
{
	data->SetCurrentFramePrevious();
}

void ModelAnimationDialog::OnFrame()
{
	data->SetCurrentFrame(GetInt(""));
}

void ModelAnimationDialog::OnAddFrame()
{
	data->AnimationDuplicateCurrentFrame();
}

void ModelAnimationDialog::OnDeleteFrame()
{
	data->AnimationDeleteCurrentFrame();
}

void ModelAnimationDialog::OnName()
{
	data->move->Name = GetString("");
}

void ModelAnimationDialog::OnFpsConst()
{
	data->move->FramesPerSecConst = GetFloat("");
}

void ModelAnimationDialog::OnFpsFactor()
{
	data->move->FramesPerSecFactor = GetFloat("");
}

void ModelAnimationDialog::OnSpeed()
{
	data->TimeScale = GetFloat("") / 100.0f;
}

void ModelAnimationDialog::OnParameter()
{
	data->TimeParam = GetFloat("");
}

void ModelAnimationDialog::OnTabControl()
{
	int s = GetInt("");
	data->Playing = (s == 2) && (data->move->Frame.num > 0);
	data->SimFrame = 0;
	data->UpdateAnimation();
}

void ModelAnimationDialog::OnUpdate(Observable *o)
{
	LoadData();
}

