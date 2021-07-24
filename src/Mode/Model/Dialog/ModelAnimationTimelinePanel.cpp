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
#include "../../../lib/math/vec2.h"

namespace hui {
	void get_style_colors(Panel *p, const string &id, Map<string,color> &colors);
}


ModelAnimationTimelinePanel::ModelAnimationTimelinePanel() {
	add_drawing_area("!height=80,noexpandy", 0, 0, "area");
	time_scale = 100;
	time_offset = -5;

	hover = -1;

	event_xp("area", "hui:draw", std::bind(&ModelAnimationTimelinePanel::on_draw, this, std::placeholders::_1));
	event_x("area", "hui:left-button-down", std::bind(&ModelAnimationTimelinePanel::on_left_button_down, this));
	event_x("area", "hui:left-button-up", std::bind(&ModelAnimationTimelinePanel::on_left_button_up, this));
	event_x("area", "hui:mouse-move", std::bind(&ModelAnimationTimelinePanel::on_mouse_move, this));
	event_x("area", "hui:mouse-wheel", std::bind(&ModelAnimationTimelinePanel::on_mouse_wheel, this));

	mode_model_animation->data->subscribe(this, [=]{ redraw("area"); });
	mode_model_animation->state.subscribe(this, [=]{ redraw("area"); });

	default_parasite = new TimeLineParasite;
	set_parasite(NULL);
}

ModelAnimationTimelinePanel::~ModelAnimationTimelinePanel() {
	mode_model_animation->data->unsubscribe(this);
	mode_model_animation->state.unsubscribe(this);
	delete default_parasite;
}

float ModelAnimationTimelinePanel::screen2sample(float x)
{	return (x / time_scale + time_offset);	}

float ModelAnimationTimelinePanel::sample2screen(float x)
{	return (x - time_offset) * time_scale;	}

string ModelAnimationTimelinePanel::get_time_str_fuzzy(double t, double dt) {
	bool sign = (t < 0);
	if (sign)
		t = -t;
	int _sec = (int)t;
	int _usec = ( ((int)(t * 1000)) %1000);
	if (dt < 1.0) {
		return format("%s%.2d,%.3d",sign?"-":"",_sec,_usec);
	} else {
		return format("%s%.2d",sign?"-":"",_sec);
	}
}

void ModelAnimationTimelinePanel::on_draw(Painter *c) {
	double MIN_GRID_DIST = 10.0;

	Map<string,color> colors;
	hui::get_style_colors(this, "area", colors);

	color bg = colors["insensitive_bg_color"];
	color fg = colors["fg_color"];
	color color_grid = color::interpolate(fg, bg, 0.4f);//color(1, 0.75f, 0.75f, 0.75f);

	c->set_line_width(0.8f);
	c->set_color(bg);
	c->draw_rect(c->area());
	c->set_font_size(8);

	rect r = rect(0, c->width, 0, c->height);
	double dt = MIN_GRID_DIST / time_scale; // >= 10 pixel
	double exp_s = ceil(log10(dt));
	double exp_s_mod = exp_s - log10(dt);
	dt = pow(10, exp_s);
//	double dw = dl * a->view_zoom;
	int nx0 = floor(screen2sample(r.x1 - 1) / dt);
	int nx1 = ceil(screen2sample(r.x2) / dt);
	color c1 = color::interpolate(bg, color_grid, exp_s_mod);
	color c2 = color_grid;
	for (int n=nx0;n<nx1;n++) {
		c->set_color(((n % 10) == 0) ? c2 : c1);
		float xx = (float)sample2screen(n * dt);
		c->draw_line({xx, 0}, {xx, (float)c->height});
	}
	c->set_color(fg);
	for (int n=nx0;n<nx1;n++) {
		if ((sample2screen(dt) - sample2screen(0)) > 30) {
			if ((((n % 10) % 3) == 0) && ((n % 10) != 9) && ((n % 10) != -9))
				c->draw_str({sample2screen(n * dt) + 2, r.y1}, get_time_str_fuzzy(n * dt, dt * 3));
		} else {
			if ((n % 10) == 0)
				c->draw_str({sample2screen(n * dt) + 2, r.y1}, get_time_str_fuzzy(n * dt, dt * 10));
		}
	}

	ModelMove *move = mode_model_animation->cur_move();
	float dur = move->duration();
	c->set_color(color(0.15f, 0.2f, 0.2f, 1));
	c->draw_rect(rect(sample2screen(0), sample2screen(dur), r.y1, r.y2));

	float t = 0;
	foreachi(ModelFrame &f, move->frame, i) {
		c->set_color((i == mode_model_animation->current_frame) ? Red : fg);
		c->set_line_width((i == hover) ? 5.0f : 2.2f);
		float x = sample2screen(t);
		c->draw_line({x, r.y1}, {x, r.y2});
		t += f.duration;
	}
	{
		c->set_color(fg);
		c->set_line_width((move->frame.num == hover) ? 5.0f : 2.2f);
		float x = sample2screen(t);
		c->set_line_dash({10,10}, 0);
		c->draw_line({x, r.y1}, {x, r.y2});
		c->set_line_dash({}, 0);
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

	if (mode_model_animation->playing) {
		c->set_line_width(1.5f);
		c->set_color(Green);
		float x = sample2screen(mode_model_animation->sim_frame_time);
		c->draw_line({x, r.y1}, {x, r.y2});
	}

	parasite->on_timeline_draw(c);
}

void ModelAnimationTimelinePanel::on_mouse_move() {
	mx = hui::GetEvent()->m.x;
	if (hui::GetEvent()->lbut) {
		if (hover > 0) {

			float t = 0;
			foreachi(ModelFrame &f, mode_model_animation->cur_move()->frame, i) {
				if (i == hover - 1)
					break;
				t += f.duration;
			}
			float dur = max(screen2sample(mx) - t, 0.0f);

			mode_model_animation->data->animationSetFrameDuration(mode_model_animation->current_move, hover - 1, dur);
		}
	} else {
		hover = get_hover();
		redraw("area");
	}

	parasite->on_timeline_mouse_move();
}

void ModelAnimationTimelinePanel::on_left_button_down() {
	if (hover >= 0 and hover < mode_model_animation->cur_move()->frame.num)
		mode_model_animation->set_current_frame(hover);


	parasite->on_timeline_left_button_down();
}

void ModelAnimationTimelinePanel::on_left_button_up() {
	parasite->on_timeline_left_button_up();
}

void ModelAnimationTimelinePanel::on_mouse_wheel() {
	mx = hui::GetEvent()->m.x;
	float time_scale_new = min(time_scale * pow(1.1f, hui::GetEvent()->scroll.y), 1000.0f);
	time_offset += mx * (1.0f / time_scale - 1.0f / time_scale_new);
	time_scale = time_scale_new;
	//UpdateTimePos();
	redraw("area");
}

int ModelAnimationTimelinePanel::get_hover() {
	float t = 0;
	float R = 5;
	foreachi(ModelFrame &f, mode_model_animation->cur_move()->frame, i){
		float x = sample2screen(t);
		if (fabs(mx - x) < R)
			return i;
		t += f.duration;
	}
	{
		float x = sample2screen(t);
		if (fabs(mx - x) < R)
			return mode_model_animation->cur_move()->frame.num;
	}
	return -1;
}

void ModelAnimationTimelinePanel::set_parasite(TimeLineParasite* p) {
	parasite = (p ? p : default_parasite);
}
