/*
 * ModeModelAnimationInterpolateFrames.cpp
 *
 *  Created on: 10.01.2015
 *      Author: michi
 */

#include "ModeModelAnimationInterpolateFrames.h"
#include "../ModeModelAnimation.h"
#include "../../dialog/ModelAnimationTimelinePanel.h"
#include "../../../../lib/math/vec2.h"

ModeModelAnimationInterpolateFrames::ModeModelAnimationInterpolateFrames(ModeModelAnimation* _parent) :
	ModeCreation<ModeModelAnimation, DataModel>("ModelMeshAnimationInterpolateFrames", _parent)
{
	message = _("click on timeline");
}

void ModeModelAnimationInterpolateFrames::on_timeline_mouse_move() {
	mode_model_animation->timeline->redraw("area");
}

void ModeModelAnimationInterpolateFrames::on_timeline_left_button_up() {
	float mx = mode_model_animation->timeline->mx;
	float x = mode_model_animation->timeline->screen2sample(mx);
	ModelMove *m = mode_model_animation->cur_move();
	if ((x >= 0) and (x < m->duration())) {
		int f0, f1;
		float t;
		m->getTimeInterpolation(x, f0, f1, t);
		data->begin_action_group("AnimationFrameInterpolation");

		ModelFrame f = m->interpolate(x);
		f.duration = m->frame[f0].duration * (1 - t);

		data->animationAddFrame(mode_model_animation->current_move, f0 + 1, f);
		data->animationSetFrameDuration(mode_model_animation->current_move, f0, m->frame[f0].duration * t);

		data->end_action_group();
	}
}

void ModeModelAnimationInterpolateFrames::on_start() {
	mode_model_animation->timeline->set_parasite(this);
}

void ModeModelAnimationInterpolateFrames::on_end() {
	mode_model_animation->timeline->set_parasite(nullptr);
}

void ModeModelAnimationInterpolateFrames::on_draw_win(MultiView::Window* win) {
	parent->on_draw_win(win);
}

void ModeModelAnimationInterpolateFrames::on_timeline_draw(Painter *p) {
	float mx = mode_model_animation->timeline->mx;
	float x = mode_model_animation->timeline->screen2sample(mx);
	p->set_line_width(2);
	if ((x >= 0) and (x < mode_model_animation->cur_move()->duration()))
		p->set_color(Green);
	else
		p->set_color(Gray);
	p->draw_line({mx, 0}, {mx, (float)p->width});
	p->draw_str({mx, 20}, mode_model_animation->timeline->get_time_str_fuzzy(x, 0.001f));
}
