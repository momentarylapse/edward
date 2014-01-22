/*
 * SingleData.cpp
 *
 *  Created on: 22.01.2014
 *      Author: michi
 */

#include "SingleData.h"
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

bool SingleData::Hover(Window *win, vector &m, vector &tp, float &z, void *user_data)
{
	vector p = win->Project(pos);
	if ((p.z <= 0) || (p.z >= 1))
		return false;
	const float _radius = 4;
	if ((m.x >= p.x - _radius) && (m.x <= p.x + _radius) && (m.y >= p.y - _radius) && (m.y <= p.y + _radius)){
		z = p.z;
		tp = pos;
		return true;
	}
	return false;
}

bool SingleData::InRect(Window *win, rect &r, void *user_data)
{
	vector p = win->Project(pos);
	return r.inside(p.x, p.y);
}

};

