/*
 * SingleData.h
 *
 *  Created on: 22.01.2014
 *      Author: michi
 */

#ifndef SINGLEDATA_H_
#define SINGLEDATA_H_


#include "../lib/math/vec3.h"

struct rect;
struct vec2;


namespace multiview{

class Window;

// "des Pudels Kern", don't change!!!!!!!
struct SingleData {
	vec3 pos = vec3::ZERO;
	int view_stage = -1;
	bool is_special = false;
};

};



#endif /* SINGLEDATA_H_ */
