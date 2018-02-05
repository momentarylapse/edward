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
	addDrawingArea("!height=120,noexpandy", 0, 0, "area");
	time_scale = 100;
	time_offset = -5;

	hover = -1;

	eventXP("area", "hui:draw", std::bind(&ModelAnimationTimelinePanel::onDraw, this, std::placeholders::_1));
	eventX("area", "hui:left-button-down", std::bind(&ModelAnimationTimelinePanel::onLeftButtonDown, this));
	eventX("area", "hui:left-button-up", std::bind(&ModelAnimationTimelinePanel::onLeftButtonUp, this));
	eventX("area", "hui:mouse-move", std::bind(&ModelAnimationTimelinePanel::onMouseMove, this));
	eventX("area", "hui:mouse-wheel", std::bind(&ModelAnimationTimelinePanel::onMouseWheel, this));

	subscribe(mode_model_animation->data);
	subscribe(mode_model_animation);

	default_parasite = new TimeLineParasite;
	setParasite(NULL);
}

ModelAnimationTimelinePanel::~ModelAnimationTimelinePanel()
{
	unsubscribe(mode_model_animation->data);
	unsubscribe(mode_model_animation);
	delete(default_parasite);
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
	int _sec = (int)t;
	int _usec = ( ((int)(t * 1000)) %1000);
	if (dt < 1.0){
		return format("%s%.2d,%.3d",sign?"-":"",_sec,_usec);
	}else{
		return format("%s%.2d",sign?"-":"",_sec);
	}
}

void ModelAnimationTimelinePanel::onDraw(Painter *c)
{
	double MIN_GRID_DIST = 10.0;
	color bg = White;
	color ColorGrid = color(1, 0.75f, 0.75f, 0.75f);

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

	ModelMove *move = mode_model_animation->cur_move();
	float dur = move->duration();
	c->setColor(color(0.15f, 0, 0, 1));
	c->drawRect(rect(sample2screen(0), sample2screen(dur), r.y1, r.y2));

	float t = 0;
	foreachi(ModelFrame &f, move->frame, i){
		c->setColor((i == mode_model_animation->current_frame) ? Red : Black);
		c->setLineWidth((i == hover) ? 5.0f : 2.2f);
		float x = sample2screen(t);
		c->drawLine(x, r.y1, x, r.y2);
		t += f.duration;
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
		float x = sample2screen(mode_model_animation->sim_frame_time);
		c->drawLine(x, r.y1, x, r.y2);
	}

	parasite->onTimelineDraw(c);
}

void ModelAnimationTimelinePanel::onMouseMove()
{
	mx = hui::GetEvent()->mx;
	if (hui::GetEvent()->lbut){
		if (hover > 0){

			float t = 0;
			foreachi(ModelFrame &f, mode_model_animation->cur_move()->frame, i){
				if (i == hover - 1)
					break;
				t += f.duration;
			}
			float dur = max(screen2sample(mx) - t, 0.0f);

			mode_model_animation->data->animationSetFrameDuration(mode_model_animation->current_move, hover - 1, dur);
		}
	}else
		updateHover();

	parasite->onTimelineMouseMove();
}

void ModelAnimationTimelinePanel::onLeftButtonDown()
{
	if (hover >= 0)
		mode_model_animation->setCurrentFrame(hover);


	parasite->onTimelineLeftButtonDown();
}

void ModelAnimationTimelinePanel::onLeftButtonUp()
{
	parasite->onTimelineLeftButtonUp();
}

void ModelAnimationTimelinePanel::onMouseWheel()
{
	mx = hui::GetEvent()->mx;
	float time_scale_new = min(time_scale * pow(1.1f, hui::GetEvent()->scroll_y), 1000.0f);
	time_offset += mx * (1.0f / time_scale - 1.0f / time_scale_new);
	time_scale = time_scale_new;
	//UpdateTimePos();
	redraw("area");
}

void ModelAnimationTimelinePanel::onUpdate(Observable* o, const string& message)
{
	redraw("area");
}

void ModelAnimationTimelinePanel::updateHover()
{
	hover = -1;
	float t = 0;
	foreachi(ModelFrame &f, mode_model_animation->cur_move()->frame, i){
		float x = sample2screen(t);
		if (fabs(mx - x) < 5)
			hover = i;
		t += f.duration;
	}
	redraw("area");
}

void ModelAnimationTimelinePanel::setParasite(TimeLineParasite* p)
{
	parasite = (p ? p : default_parasite);
}
