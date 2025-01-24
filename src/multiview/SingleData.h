/*
 * SingleData.h
 *
 *  Created on: 22.01.2014
 *      Author: michi
 */

#ifndef SINGLEDATA_H_
#define SINGLEDATA_H_


#include "../lib/math/vec3.h"

class rect;
class vec2;


namespace multiview{

class Window;

// "des Pudels Kern", don't change!!!!!!!
struct SingleData {
	SingleData();
	int view_stage;
	bool is_selected, m_delta, m_old, is_special;
	vec3 pos;
	/*virtual float hover_distance(Window *win, const vec2 &m, vec3 &tp, float &z);
	virtual bool overlap_rect(Window *win, const rect &r);
	virtual bool in_rect(Window *win, const rect &r);*/
};

};



#endif /* SINGLEDATA_H_ */
