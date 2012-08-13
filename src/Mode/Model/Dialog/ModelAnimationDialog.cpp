/*
 * ModelAnimationDialog.cpp
 *
 *  Created on: 13.08.2012
 *      Author: michi
 */

#include "ModelAnimationDialog.h"
#include "../../../Edward.h"

ModelAnimationDialog::ModelAnimationDialog(CHuiWindow *_parent, bool _allow_parent, DataModel *_data) :
	CHuiWindow("dummy", -1, -1, 230, 400, _parent, _allow_parent, HuiWinModeControls, true)
{
	data = _data;

	// dialog
	FromResource("animation_dialog");
	SetPositionSpecial(_parent, HuiRight | HuiTop);

	EventM("hui:close", this, (void(HuiEventHandler::*)())&ModelAnimationDialog::OnClose);
	EventM("animation_list", this, (void(HuiEventHandler::*)())&ModelAnimationDialog::OnAnimationList);
	EventM("animation_new", this, (void(HuiEventHandler::*)())&ModelAnimationDialog::OnAddAnimation);
	EventM("animation_delete", this, (void(HuiEventHandler::*)())&ModelAnimationDialog::OnDeleteAnimation);
	EventM("frame_inc", this, (void(HuiEventHandler::*)())&ModelAnimationDialog::OnFrameInc);
	EventM("frame_dec", this, (void(HuiEventHandler::*)())&ModelAnimationDialog::OnFrameDec);
	EventM("frame", this, (void(HuiEventHandler::*)())&ModelAnimationDialog::OnFrame);
	EventM("new_frame", this, (void(HuiEventHandler::*)())&ModelAnimationDialog::OnAddFrame);
	EventM("delete_frame", this, (void(HuiEventHandler::*)())&ModelAnimationDialog::OnDeleteFrame);
	EventM("name", this, (void(HuiEventHandler::*)())&ModelAnimationDialog::OnName);
	EventM("fps_const", this, (void(HuiEventHandler::*)())&ModelAnimationDialog::OnFpsConst);
	EventM("fps_factor", this, (void(HuiEventHandler::*)())&ModelAnimationDialog::OnFpsFactor);
	EventM("speed", this, (void(HuiEventHandler::*)())&ModelAnimationDialog::OnSpeed);
	EventM("parameter", this, (void(HuiEventHandler::*)())&ModelAnimationDialog::OnParameter);
	EventM("animation_dialog_tab_control", this, (void(HuiEventHandler::*)())&ModelAnimationDialog::OnTabControl);

	Subscribe(data);

	LoadData();
}

ModelAnimationDialog::~ModelAnimationDialog()
{
	Unsubscribe(data);
}

void ModelAnimationDialog::LoadData()
{
	Reset("animation_list");
	foreachi(data->Move, m, i)
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

void ModelAnimationDialog::OnAnimationList()
{
	int s = GetInt("");
	if (s >= 0){
		// which animation was clicked?
		int n = 0;
		foreachi(data->Move, m, i)
			if (m.Frame.num > 0){
				if (n == s)
					data->SetCurrentMove(i);
				n ++;
			}
		SetInt("animation_dialog_tab_control", 1);
	}
}

void ModelAnimationDialog::OnClose()
{
}

void ModelAnimationDialog::ApplyData()
{
}

void ModelAnimationDialog::OnAddAnimation()
{
}

void ModelAnimationDialog::OnDeleteAnimation()
{
}

void ModelAnimationDialog::OnFrameInc()
{
	data->SetCurrentFrame((data->CurrentFrame + 1) % data->move->Frame.num);
}

void ModelAnimationDialog::OnFrameDec()
{
	data->SetCurrentFrame((data->CurrentFrame + data->move->Frame.num - 1) % data->move->Frame.num);
}

void ModelAnimationDialog::OnFrame()
{
	data->SetCurrentFrame(GetInt(""));
}

void ModelAnimationDialog::OnAddFrame()
{
}

void ModelAnimationDialog::OnDeleteFrame()
{
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
	data->Playing = (GetInt("") == 2) && (data->move->Frame.num > 0);
	data->SimFrame = 0;
}

void ModelAnimationDialog::OnUpdate(Observable *o)
{
	LoadData();
}

