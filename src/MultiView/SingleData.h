/*
 * SingleData.h
 *
 *  Created on: 22.01.2014
 *      Author: michi
 */

#ifndef SINGLEDATA_H_
#define SINGLEDATA_H_


#include "../lib/math/vector.h"

class rect;

namespace MultiView{

class Window;

// "des Pudels Kern", don't change!!!!!!!
class SingleData {
public:
	SingleData();
	virtual ~SingleData() {}
	int view_stage;
	bool is_selected, m_delta, m_old, is_special;
	vector pos;
	virtual float hover_distance(Window *win, const vector &m, vector &tp, float &z);
	virtual bool overlap_rect(Window *win, const rect &r);
	virtual bool in_rect(Window *win, const rect &r);
};

};



#endif /* SINGLEDATA_H_ */
