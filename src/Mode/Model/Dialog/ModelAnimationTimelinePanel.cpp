/*
 * ModelAnimationTimelinePanel.cpp
 *
 *  Created on: 23.12.2014
 *      Author: michi
 */

#include "ModelAnimationTimelinePanel.h"
#include "../Animation/ModeModelAnimation.h"
#include "../../../Data/Model/DataModel.h"
#include "../../../Edward.h"
#include "../../../MultiView/MultiView.h"

ModelAnimationTimelinePanel::ModelAnimationTimelinePanel() :
	Observer("ModelAnimationTimelinePanel")
{
	addDrawingArea("!height=120,noexpandy", 0, 0, 0, 0, "area");
	time_scale = 100;
	time_offset = -5;

	hover = -1;

	eventX("area", "hui:draw", this, &ModelAnimationTimelinePanel::onDraw);
	eventX("area", "hui:left-button-down", this, &ModelAnimationTimelinePanel::onLeftButtonDown);
	eventX("area", "hui:left-button-up", this, &ModelAnimationTimelinePanel::onLeftButtonUp);
	eventX("area", "hui:mouse-move", this, &ModelAnimationTimelinePanel::onMouseMove);
	eventX("area", "hui:mouse-wheel", this, &ModelAnimationTimelinePanel::onMouseWheel);

	subscribe(mode_model_animation->data);
	subscribe(mode_model_animation);
}

ModelAnimationTimelinePanel::~ModelAnimationTimelinePanel()
{
	unsubscribe(mode_model_animation->data);
	unsubscribe(mode_model_animation);
}

float ModelAnimationTimelinePanel::screen2sample(float x)
{	return (x / time_scale + time_offset);	}

float ModelAnimationTimelinePanel::sample2screen(float x)
{	return (x - time_offset) * time_scale;	}

string ModelAnimationTimelinePanel::get_time_str_fuzzy(double t, double dt)
{
	bool sign = (t < 0);
	if (sign)
		t = -t;
	int _min=((int)t/60);
	int _sec=((int)t %60);
	int _usec=( ((int)(t * 1000)) %1000);
	if (dt < 1.0){
		if (_min > 0)
			return format("%s%d:%.2d,%.3d",sign?"-":"",_min,_sec,_usec);
		else
			return format("%s%.2d,%.3d",sign?"-":"",_sec,_usec);
	}else{
		if (_min > 0)
			return format("%s%d:%.2d",sign?"-":"",_min,_sec);
		else
			return format("%s%.2d",sign?"-":"",_sec);
	}
}

void ModelAnimationTimelinePanel::onDraw()
{
	double MIN_GRID_DIST = 10.0;
	color bg = White;
	color ColorGrid = color(1, 0.75f, 0.75f, 0.75f);

	HuiPainter *c = beginDraw("area");
	c->setLineWidth(0.8f);
	c->setColor(bg);
	c->drawRect(0, 0, c->width, c->height);
	c->setFontSize(8);

	rect r = rect(0, c->width, 0, c->height);
	double dt = MIN_GRID_DIST / time_scale; // >= 10 pixel
	double exp_s = ceil(log10(dt));
	double exp_s_mod = exp_s - log10(dt);
	dt = pow(10, exp_s);
//	double dw = dl * a->view_zoom;
	int nx0 = floor(screen2sample(r.x1 - 1) / dt);
	int nx1 = ceil(screen2sample(r.x2) / dt);
	color c1 = ColorInterpolate(bg, ColorGrid, exp_s_mod);
	color c2 = ColorGrid;
	for (int n=nx0;n<nx1;n++){
		c->setColor(((n % 10) == 0) ? c2 : c1);
		int xx = sample2screen(n * dt);
		c->drawLine(xx, 0, xx, c->height);
	}
	c->setColor(ColorGrid);
	for (int n=nx0;n<nx1;n++){
		if ((sample2screen(dt) - sample2screen(0)) > 30){
			if ((((n % 10) % 3) == 0) && ((n % 10) != 9) && ((n % 10) != -9))
				c->drawStr(sample2screen(n * dt) + 2, r.y1, get_time_str_fuzzy(n * dt, dt * 3));
		}else{
			if ((n % 10) == 0)
				c->drawStr(sample2screen(n * dt) + 2, r.y1, get_time_str_fuzzy(n * dt, dt * 10));
		}
	}

	ModelMove *move = mode_model_animation->move;
	float dur = move->frame.num;
	c->setColor(color(0.15f, 0, 0, 1));
	c->drawRect(rect(sample2screen(0), sample2screen(dur), r.y1, r.y2));

	foreachi(ModelFrame &f, move->frame, i){
		c->setColor((i == mode_model_animation->current_frame) ? Red : Black);
		c->setLineWidth((i == hover) ? 5.0f : 2.2f);
		float pos = sample2screen(i);
		c->drawLine(pos, r.y1, pos, r.y2);
	}

	/*Array<float> v;
	const int N = 1000;
	v.resize(N+1);
	float v_max = 0;
	for (int i=0; i<=N; i++){
		float vv = mode->inter_pos->getTang((float)i / (float)N).length();
		if (vv > v_max)
			v_max = vv;
		v[i] = vv;
	}
	c->setLineWidth(1.0f);
	c->setColor(Green);
	for (int i=0; i<N; i++){
		float t0 = (float)i / (float)N * dur;
		float t1 = (float)(i+1) / (float)N * dur;
		c->drawLine(sample2screen(t0), r.y2 - v[i]/v_max * r.height(), sample2screen(t1), r.y2 - v[i+1]/v_max * r.height());
	}*/

	if (mode_model_animation->playing){
		c->setLineWidth(1.5f);
		c->setColor(Green);
		float x = sample2screen(mode_model_animation->sim_frame);
		c->drawLine(x, r.y1, x, r.y2);
	}
	c->end();
}

void ModelAnimationTimelinePanel::onMouseMove()
{
	updateHover(HuiGetEvent()->mx);
}

void ModelAnimationTimelinePanel::onLeftButtonDown()
{
	if (hover >= 0)
		mode_model_animation->setCurrentFrame(hover);
}

void ModelAnimationTimelinePanel::onLeftButtonUp()
{
}

void ModelAnimationTimelinePanel::onMouseWheel()
{
	float time_scale_new = min(time_scale * pow(1.1f, HuiGetEvent()->dz), 1000);
	time_offset += HuiGetEvent()->mx * (1.0f / time_scale - 1.0f / time_scale_new);
	time_scale = time_scale_new;
	//UpdateTimePos();
	redraw("area");
}

void ModelAnimationTimelinePanel::onUpdate(Observable* o, const string& message)
{
	redraw("area");
}

void ModelAnimationTimelinePanel::updateHover(float mx)
{
	hover = -1;
	foreachi(ModelFrame &f, mode_model_animation->move->frame, i){
		float x = sample2screen(i);
		if (fabs(mx - x) < 5)
			hover = i;
	}
	redraw("area");
}
