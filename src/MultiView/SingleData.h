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

// "des Pudels Kern", don't change!!!!!!!
class SingleData
{
public:
	SingleData();
	int view_stage;
	bool is_selected, m_delta, m_old, is_special;
	vector pos;
};

};



#endif /* SINGLEDATA_H_ */
