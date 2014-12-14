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
	EmbeddedDialog(_parent, "animation_dialog", "root-table", 1, 0, "noexpandx"),
	Observer("ModelAnimationDialog")
{
	data = _data;

	// dialog

	win->event("hui:close", this, &ModelAnimationDialog::OnClose);
	win->event("animation_list", this, &ModelAnimationDialog::OnAnimationList);
	win->eventX("animation_list", "hui:select", this, &ModelAnimationDialog::OnAnimationListSelect);
	win->event("animation_new", this, &ModelAnimationDialog::OnAddAnimation);
	win->event("animation_delete", this, &ModelAnimationDialog::OnDeleteAnimation);
	win->event("frame", this, &ModelAnimationDialog::OnFrame);
	win->event("new_frame", this, &ModelAnimationDialog::OnAddFrame);
	win->event("delete_frame", this, &ModelAnimationDialog::OnDeleteFrame);
	win->event("name", this, &ModelAnimationDialog::OnName);
	win->event("fps_const", this, &ModelAnimationDialog::OnFpsConst);
	win->event("fps_factor", this, &ModelAnimationDialog::OnFpsFactor);
	win->event("speed", this, &ModelAnimationDialog::OnSpeed);
	win->event("parameter", this, &ModelAnimationDialog::OnParameter);
	win->event("sim_start", this, &ModelAnimationDialog::OnSimulationPlay);
	win->event("sim_stop", this, &ModelAnimationDialog::OnSimulationStop);

	subscribe(data);
	subscribe(mode_model_animation);

	LoadData();
}

ModelAnimationDialog::~ModelAnimationDialog()
{
	unsubscribe(mode_model_animation);
	unsubscribe(data);
}

void ModelAnimationDialog::LoadData()
{
	reset("animation_list");
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
			addString("animation_list", str);
			if (i == mode_model_animation->CurrentMove)
				setInt("animation_list", n);
			n ++;
		}
	FillAnimation();
	setFloat("speed", mode_model_animation->TimeScale * 100.0f);
	setFloat("parameter", mode_model_animation->TimeParam);
}

void ModelAnimationDialog::FillAnimation()
{
	bool b = mode_model_animation->move->Frame.num > 0;
	enable("name", b);
	enable("frame", b);
	enable("frame_inc", b);
	enable("frame_dec", b);
	enable("new_frame", b);
	enable("delete_frame", b);
	enable("fps_const", b);
	enable("fps_factor", b);
	if (b){
		ModelMove *move = mode_model_animation->move;
		setString("name", move->Name);
		setInt("frame", mode_model_animation->CurrentFrame);
		setFloat("fps_const", move->FramesPerSecConst);
		setFloat("fps_factor", move->FramesPerSecFactor);
	}
}

int ModelAnimationDialog::GetSelectedAnimation()
{
	int s = getInt("animation_list");
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
		setInt("animation_dialog_tab_control", 1);
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
	// first free index
	int index = data->Move.num;
	foreachi(ModelMove &m, data->Move, i)
		if (m.Frame.num == 0){
			index = i;
			break;
		}

	ModelNewAnimationDialog *dlg = new ModelNewAnimationDialog(win, false, data, index);
	dlg->run();
}

void ModelAnimationDialog::OnDeleteAnimation()
{
	int s = GetSelectedAnimation();
	if (s >= 0)
		data->DeleteAnimation(s);
}

void ModelAnimationDialog::OnFrame()
{
	int frame_lit = getInt("");
	int frame = loopi(frame_lit, 0, mode_model_animation->move->Frame.num - 1);
	if (frame != frame_lit)
		setInt("", frame);
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
	mode_model_animation->move->Name = getString("");
}

void ModelAnimationDialog::OnFpsConst()
{
	mode_model_animation->move->FramesPerSecConst = getFloat("");
}

void ModelAnimationDialog::OnFpsFactor()
{
	mode_model_animation->move->FramesPerSecFactor = getFloat("");
}

void ModelAnimationDialog::OnSpeed()
{
	mode_model_animation->TimeScale = getFloat("") / 100.0f;
}

void ModelAnimationDialog::OnParameter()
{
	mode_model_animation->TimeParam = getFloat("");
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

void ModelAnimationDialog::onUpdate(Observable *o, const string &message)
{
	if (o == data){
		LoadData();
	}else{
		FillAnimation();
	}
}

