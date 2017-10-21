/*
 * ModeModelAnimationInterpolateFrames.cpp
 *
 *  Created on: 10.01.2015
 *      Author: michi
 */

#include "ModeModelAnimationInterpolateFrames.h"
#include "../ModeModelAnimation.h"
#include "../../Dialog/ModelAnimationTimelinePanel.h"

ModeModelAnimationInterpolateFrames::ModeModelAnimationInterpolateFrames(ModeBase* _parent) :
	ModeCreation<DataModel>("ModelMeshAnimationInterpolateFrames", _parent)
{
	message = _("auf Zeitleiste klicken");
}

ModeModelAnimationInterpolateFrames::~ModeModelAnimationInterpolateFrames()
{
}

void ModeModelAnimationInterpolateFrames::onTimelineMouseMove()
{
	mode_model_animation->timeline->redraw("area");
}

void ModeModelAnimationInterpolateFrames::onTimelineLeftButtonUp()
{
	float mx = mode_model_animation->timeline->mx;
	float x = mode_model_animation->timeline->screen2sample(mx);
	ModelMove *m = mode_model_animation->cur_move();
	if ((x >= 0) and (x < m->duration())){
		int f0, f1;
		float t;
		m->getTimeInterpolation(x, f0, f1, t);
		data->beginActionGroup("AnimationFrameInterpolation");

		ModelFrame f = m->interpolate(x);
		f.duration = m->frame[f0].duration * (1 - t);

		data->animationAddFrame(mode_model_animation->current_move, f0 + 1, f);
		data->animationSetFrameDuration(mode_model_animation->current_move, f0, m->frame[f0].duration * t);

		data->endActionGroup();
	}
}

void ModeModelAnimationInterpolateFrames::onStart()
{
	mode_model_animation->timeline->setParasite(this);
}

void ModeModelAnimationInterpolateFrames::onEnd()
{
	mode_model_animation->timeline->setParasite(NULL);
}

void ModeModelAnimationInterpolateFrames::onDrawWin(MultiView::Window* win)
{
	parent->onDrawWin(win);
}

void ModeModelAnimationInterpolateFrames::onTimelineDraw(Painter *p)
{
	float mx = mode_model_animation->timeline->mx;
	float x = mode_model_animation->timeline->screen2sample(mx);
	p->setLineWidth(2);
	if ((x >= 0) and (x < mode_model_animation->cur_move()->duration()))
		p->setColor(Green);
	else
		p->setColor(Gray);
	p->drawLine(mx, 0, mx, p->width);
	p->drawStr(mx, 20, mode_model_animation->timeline->get_time_str_fuzzy(x, 0.001f));
}
