/*
 * ModelAnimationDialog.cpp
 *
 *  Created on: 13.08.2012
 *      Author: michi
 */

#include "ModelAnimationDialog.h"
#include "../../../Data/Model/DataModel.h"
#include "ModelNewAnimationDialog.h"
#include "ModelDuplicateAnimationDialog.h"
#include "../Animation/ModeModelAnimation.h"


ModelAnimationDialog::ModelAnimationDialog(DataModel *_data) :
	Observer("ModelAnimationDialog")
{
	fromResource("animation_dialog");
	data = _data;

	// dialog

	//event("hui:close", std::bind(&ModelAnimationDialog::onClose);
	event("animation_list", std::bind(&ModelAnimationDialog::onAnimationList, this));
	eventX("animation_list", "hui:select", std::bind(&ModelAnimationDialog::onAnimationListSelect, this));
	event("animation_new", std::bind(&ModelAnimationDialog::onAddAnimation, this));
	event("animation_delete", std::bind(&ModelAnimationDialog::onDeleteAnimation, this));
	event("animation_copy", std::bind(&ModelAnimationDialog::onCopyAnimation, this));
	event("frame", std::bind(&ModelAnimationDialog::onFrame, this));
	event("new_frame", std::bind(&ModelAnimationDialog::onAddFrame, this));
	event("delete_frame", std::bind(&ModelAnimationDialog::onDeleteFrame, this));
	event("name", std::bind(&ModelAnimationDialog::onName, this));
	event("fps_const", std::bind(&ModelAnimationDialog::onFpsConst, this));
	event("fps_factor", std::bind(&ModelAnimationDialog::onFpsFactor, this));
	event("speed", std::bind(&ModelAnimationDialog::onSpeed, this));
	event("parameter", std::bind(&ModelAnimationDialog::onParameter, this));
	event("sim_start", std::bind(&ModelAnimationDialog::onSimulationPlay, this));
	event("sim_stop", std::bind(&ModelAnimationDialog::onSimulationStop, this));

	subscribe(data);
	subscribe(mode_model_animation);

	loadData();
}

ModelAnimationDialog::~ModelAnimationDialog()
{
	unsubscribe(mode_model_animation);
	unsubscribe(data);
}

void ModelAnimationDialog::loadData()
{
	reset("animation_list");
	int n = 0;
	foreachi(ModelMove &m, data->move, i)
		if (m.frame.num > 0){
			string str = i2s(i) + "\\";
			if (m.type == MOVE_TYPE_VERTEX)
				str += _("Vertex");
			else if (m.type == MOVE_TYPE_SKELETAL)
				str += _("Skelett");
			else
				str += "???";
			str += format("\\%d\\", m.frame.num) + m.name;
			addString("animation_list", str);
			if (i == mode_model_animation->current_move)
				setInt("animation_list", n);
			n ++;
		}
	fillAnimation();
	setFloat("speed", mode_model_animation->time_scale * 100.0f);
	setFloat("parameter", mode_model_animation->time_param);
}

void ModelAnimationDialog::fillAnimation()
{
	bool b = (mode_model_animation->cur_move()->type != MOVE_TYPE_NONE);
	enable("name", b);
	enable("frame", b);
	enable("new_frame", b);
	enable("delete_frame", b);
	enable("fps_const", b);
	enable("fps_factor", b);
	enable("sim_start", b);
	enable("sim_stop", b);
	if (b){
		ModelMove *move = mode_model_animation->cur_move();
		setString("name", move->name);
		setInt("frame", mode_model_animation->current_frame);
		setFloat("fps_const", move->frames_per_sec_const);
		setFloat("fps_factor", move->frames_per_sec_factor);
	}
}

void ModelAnimationDialog::onCopyAnimation()
{
	int index = getFirstFreeIndex();

	ModelDuplicateAnimationDialog *dlg = new ModelDuplicateAnimationDialog(win, false, data, index, mode_model_animation->current_move);
	dlg->run();
	delete dlg;
}

int ModelAnimationDialog::getSelectedAnimation()
{
	int s = getInt("animation_list");
	if (s >= 0){
		int n = 0;
		foreachi(ModelMove &m, data->move, i)
			if (m.frame.num > 0){
				if (n == s)
					return i;
				n ++;
			}
	}
	return -1;
}

void ModelAnimationDialog::onAnimationList()
{
	int s = getSelectedAnimation();
	mode_model_animation->setCurrentMove(s);
}

void ModelAnimationDialog::onAnimationListSelect()
{
	int s = getSelectedAnimation();
	mode_model_animation->setCurrentMove(s);
}

void ModelAnimationDialog::onClose()
{
}

void ModelAnimationDialog::applyData()
{
}

void ModelAnimationDialog::onAddAnimation()
{
	int index = getFirstFreeIndex();
	int type = (data->bone.num > 0) ? MOVE_TYPE_SKELETAL : MOVE_TYPE_VERTEX;

	ModelNewAnimationDialog *dlg = new ModelNewAnimationDialog(win, false, data, index, type);
	dlg->run();
	delete dlg;
}

void ModelAnimationDialog::onDeleteAnimation()
{
	int s = getSelectedAnimation();
	if (s >= 0)
		data->deleteAnimation(s);
}

void ModelAnimationDialog::onFrame()
{
	int frame_lit = getInt("");
	int frame = loopi(frame_lit, 0, mode_model_animation->cur_move()->frame.num - 1);
	if (frame != frame_lit)
		setInt("", frame);
	mode_model_animation->setCurrentFrame(frame);
}

void ModelAnimationDialog::onAddFrame()
{
	mode_model_animation->duplicateCurrentFrame();
}

void ModelAnimationDialog::onDeleteFrame()
{
	mode_model_animation->deleteCurrentFrame();
}

void ModelAnimationDialog::onName()
{
	data->setAnimationData(mode_model_animation->current_move, getString(""), mode_model_animation->cur_move()->frames_per_sec_const, mode_model_animation->cur_move()->frames_per_sec_factor);
}

void ModelAnimationDialog::onFpsConst()
{
	data->setAnimationData(mode_model_animation->current_move, mode_model_animation->cur_move()->name, getFloat(""), mode_model_animation->cur_move()->frames_per_sec_factor);
}

void ModelAnimationDialog::onFpsFactor()
{
	data->setAnimationData(mode_model_animation->current_move, mode_model_animation->cur_move()->name, mode_model_animation->cur_move()->frames_per_sec_const, getFloat(""));
}

void ModelAnimationDialog::onSpeed()
{
	mode_model_animation->time_scale = getFloat("") / 100.0f;
}

void ModelAnimationDialog::onParameter()
{
	mode_model_animation->time_param = getFloat("");
}

void ModelAnimationDialog::onSimulationPlay()
{
	mode_model_animation->playing = (mode_model_animation->cur_move()->frame.num > 0);
	mode_model_animation->sim_frame_time = 0;
	mode_model_animation->updateAnimation();
}

void ModelAnimationDialog::onSimulationStop()
{
	mode_model_animation->playing = false;
	mode_model_animation->sim_frame_time = 0;
	mode_model_animation->updateAnimation();
}

void ModelAnimationDialog::onUpdate(Observable *o, const string &message)
{
	if (o == data){
		loadData();
	}else{
		loadData();
		//fillAnimation();
	}
}

int ModelAnimationDialog::getFirstFreeIndex()
{
	foreachi(ModelMove &m, data->move, i)
		if (m.frame.num == 0){
			return i;
		}
	return data->move.num;
}
