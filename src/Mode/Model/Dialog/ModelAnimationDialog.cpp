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


ModelAnimationDialog::ModelAnimationDialog(HuiWindow *_parent, DataModel *_data) :
	EmbeddedDialog(_parent, "animation_dialog", "root-table", 1, 0, 250, -1)
{
	data = _data;

	// dialog

	win->EventM("hui:close", this, &ModelAnimationDialog::OnClose);
	win->EventM("animation_list", this, &ModelAnimationDialog::OnAnimationList);
	win->EventMX("animation_list", "hui:select", this, &ModelAnimationDialog::OnAnimationListSelect);
	win->EventM("animation_new", this, &ModelAnimationDialog::OnAddAnimation);
	win->EventM("animation_delete", this, &ModelAnimationDialog::OnDeleteAnimation);
	win->EventM("frame", this, &ModelAnimationDialog::OnFrame);
	win->EventM("new_frame", this, &ModelAnimationDialog::OnAddFrame);
	win->EventM("delete_frame", this, &ModelAnimationDialog::OnDeleteFrame);
	win->EventM("name", this, &ModelAnimationDialog::OnName);
	win->EventM("fps_const", this, &ModelAnimationDialog::OnFpsConst);
	win->EventM("fps_factor", this, &ModelAnimationDialog::OnFpsFactor);
	win->EventM("speed", this, &ModelAnimationDialog::OnSpeed);
	win->EventM("parameter", this, &ModelAnimationDialog::OnParameter);
	win->EventM("sim_start", this, &ModelAnimationDialog::OnSimulationPlay);
	win->EventM("sim_stop", this, &ModelAnimationDialog::OnSimulationStop);

	Subscribe(data);
	Subscribe(mode_model_animation);

	LoadData();
}

ModelAnimationDialog::~ModelAnimationDialog()
{
	Unsubscribe(mode_model_animation);
	Unsubscribe(data);
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
	SetFloat("speed", mode_model_animation->TimeScale * 100.0f);
	SetFloat("parameter", mode_model_animation->TimeParam);
}

void ModelAnimationDialog::FillAnimation()
{
	bool b = mode_model_animation->move->Frame.num > 0;
	Enable("name", b);
	Enable("frame", b);
	Enable("frame_inc", b);
	Enable("frame_dec", b);
	Enable("new_frame", b);
	Enable("delete_frame", b);
	Enable("fps_const", b);
	Enable("fps_factor", b);
	if (b){
		ModelMove *move = mode_model_animation->move;
		SetString("name", move->Name);
		SetInt("frame", mode_model_animation->CurrentFrame);
		SetFloat("fps_const", move->FramesPerSecConst);
		SetFloat("fps_factor", move->FramesPerSecFactor);
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
	ModelNewAnimationDialog *dlg = new ModelNewAnimationDialog(win, false, data, 0);
	dlg->Run();
}

void ModelAnimationDialog::OnDeleteAnimation()
{
	int s = GetSelectedAnimation();
	if (s >= 0)
		data->DeleteAnimation(s);
}

void ModelAnimationDialog::OnFrame()
{
	int frame_lit = GetInt("");
	int frame = loopi(frame_lit, 0, mode_model_animation->move->Frame.num - 1);
	if (frame != frame_lit)
		SetInt("", frame);
	mode_model_animation->SetCurrentFrame(frame);
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

void ModelAnimationDialog::OnSimulationPlay()
{
	mode_model_animation->Playing = (mode_model_animation->move->Frame.num > 0);
	mode_model_animation->SimFrame = 0;
	mode_model_animation->UpdateAnimation();
}

void ModelAnimationDialog::OnSimulationStop()
{
	mode_model_animation->Playing = false;
	mode_model_animation->SimFrame = 0;
	mode_model_animation->UpdateAnimation();
}

void ModelAnimationDialog::OnUpdate(Observable *o)
{
	if (o->GetName() == "Data"){
		LoadData();
	}else{
		FillAnimation();
	}
}

