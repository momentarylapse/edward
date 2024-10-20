/*
 * ModelAnimationDialog.cpp
 *
 *  Created on: 13.08.2012
 *      Author: michi
 */

#include "ModelAnimationDialog.h"
#include "ModelNewAnimationDialog.h"
#include "ModelDuplicateAnimationDialog.h"
#include "../animation/ModeModelAnimation.h"
#include "../ModeModel.h"
#include "../../../Session.h"
#include "../../../data/model/DataModel.h"
#include <y/world/components/Animator.h>


ModelAnimationDialog::ModelAnimationDialog(DataModel *_data) {
	from_resource("animation_dialog");
	data = _data;
	mode_model_animation = data->session->mode_model->mode_model_animation;

	// dialog

	event("animation_list", [=]{ on_animation_list(); });
	event_x("animation_list", "hui:select", [=]{ on_animation_list_select(); });
	event_x("animation_list", "hui:right-button-down", [=]{ on_animation_list_right_click(); });
	event("animation_new", [=]{ on_add_animation(); });
	event("animation_delete", [=]{ on_delete_animation(); });
	event("animation_copy", [=]{ on_copy_animation(); });
	event("frame", [=]{ on_frame(); });
	event("new_frame", [=]{ on_add_frame(); });
	event("delete_frame", [=]{ on_delete_frame(); });
	event("name", [=]{ on_name(); });
	event("fps_const", [=]{ on_fps_const(); });
	event("fps_factor", [=]{ on_fps_factor(); });
	event("speed", [=]{ on_speed(); });
	event("parameter", [=]{ on_parameter(); });
	event("sim_start", [=]{ on_simulation_play(); });
	event("sim_stop", [=]{ on_simulation_stop(); });

	data->out_changed >> create_sink([=]{ load_data(); });
	data->out_material_changed >> create_sink([=]{ load_data(); });
	data->out_selection >> create_sink([=]{ load_data(); });
	data->out_skin_changed >> create_sink([=]{ load_data(); });
	data->out_texture_changed >> create_sink([=]{ load_data(); });
	mode_model_animation->state.out_changed >> create_sink([=]{ load_data(); });
	mode_model_animation->state.out_set_frame >> create_sink([=]{ load_data(); });
	//fillAnimation();

	popup = hui::create_resource_menu("model-animation-list-popup", this);

	load_data();
}

ModelAnimationDialog::~ModelAnimationDialog() {
	mode_model_animation->state.unsubscribe(this);
	data->unsubscribe(this);
}

void ModelAnimationDialog::load_data() {
	reset("animation_list");
	int n = 0;
	foreachi(ModelMove &m, data->move, i)
		if (m.frame.num > 0) {
			string str = i2s(i) + "\\";
			if (m.type == AnimationType::VERTEX)
				str += _("Vertex");
			else if (m.type == AnimationType::SKELETAL)
				str += _("Skeleton");
			else
				str += "???";
			str += format("\\%d\\", m.frame.num) + m.name;
			add_string("animation_list", str);
			if (i == mode_model_animation->current_move)
				set_int("animation_list", n);
			n ++;
		}
	fill_animation();
	set_float("speed", mode_model_animation->time_scale * 100.0f);
	set_float("parameter", mode_model_animation->time_param);
}

void ModelAnimationDialog::fill_animation() {
	bool b = (mode_model_animation->cur_move()->type != AnimationType::NONE);
	enable("name", b);
	enable("frame", b);
	enable("new_frame", b);
	enable("delete_frame", b);
	enable("fps_const", b);
	enable("fps_factor", b);
	enable("sim_start", b);
	enable("sim_stop", b);
	if (b) {
		ModelMove *move = mode_model_animation->cur_move();
		set_string("name", move->name);
		set_int("frame", mode_model_animation->current_frame);
		set_float("fps_const", move->frames_per_sec_const);
		set_float("fps_factor", move->frames_per_sec_factor);
	}
}

void ModelAnimationDialog::on_copy_animation() {
	int index = get_first_free_index();

	hui::fly_and_wait(new ModelDuplicateAnimationDialog(win, false, data, index, mode_model_animation->current_move));
}

int ModelAnimationDialog::get_selected_animation() {
	int s = get_int("animation_list");
	if (s >= 0) {
		int n = 0;
		foreachi(ModelMove &m, data->move, i)
			if (m.frame.num > 0) {
				if (n == s)
					return i;
				n ++;
			}
	}
	return -1;
}

void ModelAnimationDialog::on_animation_list() {
	int s = get_selected_animation();
	mode_model_animation->set_current_move(s);
}

void ModelAnimationDialog::on_animation_list_select() {
	int s = get_selected_animation();
	mode_model_animation->set_current_move(s);
}

void ModelAnimationDialog::on_animation_list_right_click() {
	int n = hui::get_event()->row;
	popup->enable("animation_copy", n >= 0);
	popup->enable("animation_delete", n >= 0);
	popup->open_popup(this);
}

void ModelAnimationDialog::on_add_animation() {
	int index = get_first_free_index();
	auto type = (data->bone.num > 0) ? AnimationType::SKELETAL : AnimationType::VERTEX;

	hui::fly_and_wait(new ModelNewAnimationDialog(win, false, data, index, type));
}

void ModelAnimationDialog::on_delete_animation() {
	int s = get_selected_animation();
	if (s >= 0)
		data->deleteAnimation(s);
}

void ModelAnimationDialog::on_frame() {
	int frame_lit = get_int("");
	int frame = loop(frame_lit, 0, mode_model_animation->cur_move()->frame.num);
	if (frame != frame_lit)
		set_int("", frame);
	mode_model_animation->set_current_frame(frame);
}

void ModelAnimationDialog::on_add_frame() {
	mode_model_animation->duplicate_current_frame();
}

void ModelAnimationDialog::on_delete_frame() {
	mode_model_animation->delete_current_frame();
}

void ModelAnimationDialog::on_name() {
	data->setAnimationData(mode_model_animation->current_move, get_string(""), mode_model_animation->cur_move()->frames_per_sec_const, mode_model_animation->cur_move()->frames_per_sec_factor);
}

void ModelAnimationDialog::on_fps_const() {
	data->setAnimationData(mode_model_animation->current_move, mode_model_animation->cur_move()->name, get_float(""), mode_model_animation->cur_move()->frames_per_sec_factor);
}

void ModelAnimationDialog::on_fps_factor() {
	data->setAnimationData(mode_model_animation->current_move, mode_model_animation->cur_move()->name, mode_model_animation->cur_move()->frames_per_sec_const, get_float(""));
}

void ModelAnimationDialog::on_speed() {
	mode_model_animation->time_scale = get_float("") / 100.0f;
}

void ModelAnimationDialog::on_parameter() {
	mode_model_animation->time_param = get_float("");
}

void ModelAnimationDialog::on_simulation_play() {
	mode_model_animation->playing = (mode_model_animation->cur_move()->frame.num > 0);
	mode_model_animation->sim_frame_time = 0;
	mode_model_animation->update_animation();
}

void ModelAnimationDialog::on_simulation_stop() {
	mode_model_animation->playing = false;
	mode_model_animation->sim_frame_time = 0;
	mode_model_animation->update_animation();
}


int ModelAnimationDialog::get_first_free_index() {
	foreachi(ModelMove &m, data->move, i)
		if (m.frame.num == 0){
			return i;
		}
	return data->move.num;
}
