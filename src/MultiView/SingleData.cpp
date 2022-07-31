/*
 * SingleData.cpp
 *
 *  Created on: 22.01.2014
 *      Author: michi
 */

#include "SingleData.h"
#include "MultiView.h"
#include "Window.h"
#include "ColorScheme.h"

namespace MultiView {

SingleData::SingleData() {
	view_stage = -1;
	is_selected = false;
	m_delta = false;
	m_old = false;
	is_special = false;
	pos = v_0;
}

float SingleData::hover_distance(Window *win, const vec2 &m, vec3 &tp, float &z) {
	vec3 p = win->project(pos);
	if ((p.z <= 0) or (p.z >= 1))
		return -1;
	z = p.z;
	tp = pos;
	return abs(p.x - m.x) + abs(p.y - m.y);
}

bool SingleData::in_rect(Window *win, const rect &r) {
	vec3 p = win->project(pos);
	return r.inside(p.xy());
}

bool SingleData::overlap_rect(Window *win, const rect &r) {
	vec3 p = win->project(pos);
	return r.inside(p.xy());
}

};

