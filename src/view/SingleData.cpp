/*
 * SingleData.cpp
 *
 *  Created on: 22.01.2014
 *      Author: michi
 */

#include "SingleData.h"
#if 0 //HAS_LIB_GL
#include "MultiView.h"
#include "Window.h"
#endif



namespace multiview {


#if 0
float SingleData::hover_distance(Window *win, const vec2 &m, vec3 &tp, float &z) {
#if HAS_LIB_GL
	vec3 p = win->project(pos);
	if ((p.z <= 0) or (p.z >= 1))
		return -1;
	z = p.z;
	tp = pos;
	return abs(p.x - m.x) + abs(p.y - m.y);
#else
	return 0;
#endif
}

bool SingleData::in_rect(Window *win, const rect &r) {
#if HAS_LIB_GL
	vec3 p = win->project(pos);
	return r.inside(p.xy());
#else
	return false;
#endif
}

bool SingleData::overlap_rect(Window *win, const rect &r) {
#if HAS_LIB_GL
	vec3 p = win->project(pos);
	return r.inside(p.xy());
#else
	return false;
#endif
}
#endif

};

