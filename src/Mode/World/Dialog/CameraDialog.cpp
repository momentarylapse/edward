/*
 * CameraDialog.cpp
 *
 *  Created on: 21.09.2013
 *      Author: michi
 */

#include "CameraDialog.h"
#include "../Camera/ModeWorldCamera.h"
#include "../../../Data/World/DataCamera.h"
#include "../../../Action/World/Camera/ActionCameraMoveTimeSelection.h"
#include "../../../lib/math/vec2.h"
#include "../../../Edward.h"
#include "../../../MultiView/MultiView.h"

CameraDialog::CameraDialog(ModeWorldCamera *_mode) {
	from_resource("world_camera_dialog");
	mode = _mode;
	data = mode->data;

	event_xp("cam_area", "hui:draw", [=](Painter *p){ OnAreaDraw(p); });
	event_x("cam_area", "hui:left-button-down", [=]{ OnAreaLeftButtonDown(); });
	event_x("cam_area", "hui:left-button-up", [=]{ OnAreaLeftButtonUp(); });
	event_x("cam_area", "hui:mouse-move", [=]{ OnAreaMouseMove(); });
	event_x("cam_area", "hui:mouse-wheel", [=]{ OnAreaMouseWheel(); });
	event("add_point", [=]{ OnAddPoint(); });
	event("delete_point", [=]{ OnDeletePoint(); });
	event("cam_edit_vel", [=]{ OnCamEditVel(); });
	event("cam_edit_ang", [=]{ OnCamEditAng(); });
	event("cam_preview", [=]{ OnCamPreview(); });
	event("cam_stop", [=]{ OnCamStop(); });

	event("hui:close", [=]{ OnCloseDialog(); });

	enable("cam_stop", false);

	hover = -1;
	mouse_distance = -1;

	time_scale = 10;
	time_offset = -5;

	mt_action = NULL;

	data->subscribe(this, [=]{ LoadData(); });
	mode->multi_view->subscribe(this, [=]{ LoadData(); });
	//mode->subscribe(this, [=]{ LoadData(); });

	LoadData();
}

CameraDialog::~CameraDialog() {
	//unsubscribe(mode->unsu);
	mode->multi_view->unsubscribe(this);
	data->unsubscribe(this);
}


void CameraDialog::OnCloseDialog() {
	ed->allow_termination([this] {
		mode->_new();
		ed->set_mode(mode->parent);
	});
}

void CameraDialog::OnAddPoint()
{
	mode->addPoint();
}

void CameraDialog::OnDeletePoint()
{
	mode->deletePoint();
}

#define screen2sample(x)	((x) / time_scale + time_offset)
#define sample2screen(x)	(((x) - time_offset) * time_scale)
static string get_time_str_fuzzy(double t, double dt)
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

void CameraDialog::UpdateTimePos()
{
	time_pos.clear();
	float t0 = 0;
	bool prev_was_flight = false;
	foreachi(WorldCamPoint &p, data->Point, i){
		float t1 = t0 + p.Duration;
		if (p.Type == CPKCamFlight){
			time_pos.add(sample2screen(t1));
		}else{
			time_pos.add(sample2screen(t0) + (prev_was_flight ? 5 : 0));
		}
		prev_was_flight = (p.Type == CPKCamFlight);
		t0 = t1;
	}
}

void CameraDialog::LoadData()
{
	UpdateTimePos();
	check("cam_edit_vel", mode->edit_vel);
	check("cam_edit_ang", mode->edit_ang);
	enable("cam_stop", mode->preview);
	redraw("cam_area");
}

void CameraDialog::OnAreaDraw(Painter *c)
{
	double MIN_GRID_DIST = 10.0;
	color bg = White;
	color ColorGrid = color(1, 0.75f, 0.75f, 0.75f);

	//HuiPainter *c = beginDraw("cam_area");
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
	color c1 = color::interpolate(bg, ColorGrid, exp_s_mod);
	color c2 = ColorGrid;
	for (int n=nx0;n<nx1;n++){
		c->set_color(((n % 10) == 0) ? c2 : c1);
		float xx = sample2screen(n * dt);
		c->draw_line({xx, 0}, {xx, (float)c->height});
	}
	c->set_color(ColorGrid);
	for (int n=nx0;n<nx1;n++){
		if ((sample2screen(dt) - sample2screen(0)) > 30){
			if ((((n % 10) % 3) == 0) && ((n % 10) != 9) && ((n % 10) != -9))
				c->draw_str({(float)sample2screen(n * dt) + 2, r.y1}, get_time_str_fuzzy(n * dt, dt * 3));
		}else{
			if ((n % 10) == 0)
				c->draw_str({(float)sample2screen(n * dt) + 2, r.y1}, get_time_str_fuzzy(n * dt, dt * 10));
		}
	}

	float dur = data->GetDuration();
	c->set_color(color(0.15f, 0, 0, 1));
	c->draw_rect(rect(sample2screen(0), sample2screen(dur), r.y1, r.y2));

	float t0 = 0;
	foreachi(WorldCamPoint &p, data->Point, i){
		c->set_color(p.is_selected ? Red : Black);
		c->set_line_width((i == hover) ? 5.0f : 2.2f);
		c->draw_line({time_pos[i], r.y1}, {time_pos[i], r.y2});
		float t1 = t0 + p.Duration;
		t0 = t1;
	}

	Array<float> v;
	const int N = 1000;
	v.resize(N+1);
	float v_max = 0;
	for (int i=0; i<=N; i++){
		float vv = mode->inter_pos->getTang((float)i / (float)N).length();
		if (vv > v_max)
			v_max = vv;
		v[i] = vv;
	}
	c->set_line_width(1.0f);
	c->set_color(Green);
	for (int i=0; i<N; i++){
		float t0 = (float)i / (float)N * dur;
		float t1 = (float)(i+1) / (float)N * dur;
		c->draw_line({sample2screen(t0), r.y2 - v[i]/v_max * r.height()}, {sample2screen(t1), r.y2 - v[i+1]/v_max * r.height()});
	}

	if (mode->preview){
		c->set_line_width(1.5f);
		c->set_color(Green);
		c->draw_line({sample2screen(mode->preview_time), r.y1}, {sample2screen(mode->preview_time), r.y2});
	}
	//c->end();
}

void CameraDialog::OnAreaLeftButtonDown()
{
	if ((hover >= 0) && (data->Point[hover].is_selected)){
		mouse_distance = 0;
		mt_time0 = screen2sample(hui::get_event()->m.x);
	}else{
		foreachi(WorldCamPoint &p, data->Point, i)
			p.is_selected = (i == hover);
		mode->multi_view->force_redraw();
		redraw("cam_area");
	}
}

void CameraDialog::OnAreaLeftButtonUp()
{
	if (mt_action){
		mt_action->undo(data);
		data->execute(mt_action);
		mt_action = NULL;
		mouse_distance = -1;
	}
}

void CameraDialog::OnAreaMouseMove()
{
	int mx = hui::get_event()->m.x;

	if (hui::get_event()->lbut){
		if (mouse_distance >= 0)
			mouse_distance += abs(hui::get_event()->d.x);
		if (mouse_distance > 5){
			if (mt_action){
				mt_action->undo(data);
				delete(mt_action);
			}
			mt_action = new ActionCameraMoveTimeSelection(data, screen2sample(mx), mt_time0);
			mt_action->execute(data);
			data->notify();
		}
	}else{
		int new_hover = -1;
		foreachi(float t, time_pos, i)
			if (fabs(mx - t) < 5)
				new_hover = i;
		if (new_hover != hover){
			hover = new_hover;
			redraw("cam_area");
		}
	}
}

void CameraDialog::OnAreaMouseWheel()
{
	float time_scale_new = min(time_scale * (float)pow(1.1f, hui::get_event()->scroll.y), 1000.0f);
	time_offset += hui::get_event()->m.x * (1.0f / time_scale - 1.0f / time_scale_new);
	time_scale = time_scale_new;
	UpdateTimePos();
	redraw("cam_area");
}


void CameraDialog::OnCamEditVel()
{
	mode->setEditVel(is_checked(""));
}

void CameraDialog::OnCamEditAng()
{
	mode->setEditAng(is_checked(""));
}

void CameraDialog::OnCamPreview()
{
	mode->previewStart();
}

void CameraDialog::OnCamStop()
{
	mode->previewStop();
}

