/*
 * SingleData.cpp
 *
 *  Created on: 22.01.2014
 *      Author: michi
 */

#include "SingleData.h"
#include "MultiView.h"
#include "Window.h"

namespace MultiView{

SingleData::SingleData()
{
	view_stage = 0;
	is_selected = false;
	m_delta = false;
	m_old = false;
	is_special = false;
	pos = v_0;
}

bool SingleData::hover(Window *win, vector &m, vector &tp, float &z, void *user_data)
{
	vector p = win->project(pos);
	if ((p.z <= 0) or (p.z >= 1))
		return false;
	float _radius = win->multi_view->POINT_RADIUS_HOVER;
	if ((m.x >= p.x - _radius) and (m.x <= p.x + _radius) and (m.y >= p.y - _radius) and (m.y <= p.y + _radius)){
		z = p.z;
		tp = pos;
		return true;
	}
	return false;
}

bool SingleData::inRect(Window *win, rect &r, void *user_data)
{
	vector p = win->project(pos);
	return r.inside(p.x, p.y);
}

};

