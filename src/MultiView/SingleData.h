/*
 * SingleData.h
 *
 *  Created on: 22.01.2014
 *      Author: michi
 */

#ifndef SINGLEDATA_H_
#define SINGLEDATA_H_


#include "../lib/math/math.h"


namespace MultiView{

class Window;

// "des Pudels Kern", don't change!!!!!!!
class SingleData
{
public:
	SingleData();
	virtual ~SingleData(){}
	int view_stage;
	bool is_selected, m_delta, m_old, is_special;
	vector pos;
	static bool default_hover(const SingleData *p, Window *win, vector &m, vector &tp, float &z, void *user_data);
	static bool default_in_rect(const SingleData *p, Window *win, rect &r, void *user_data);
	virtual bool hover(Window *win, vector &m, vector &tp, float &z, void *user_data);
	virtual bool inRect(Window *win, rect &r, void *user_data);
};

};



#endif /* SINGLEDATA_H_ */
