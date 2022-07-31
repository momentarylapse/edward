/*
 * ActionModelSplitPolygon.h
 *
 *  Created on: 01.04.2012
 *      Author: michi
 */

#ifndef ACTIONMODELSPLITPOLYGON_H_
#define ACTIONMODELSPLITPOLYGON_H_

#include "../../../ActionGroup.h"
class DataModel;
class vec3;

class ActionModelSplitPolygon: public ActionGroup {
public:
	ActionModelSplitPolygon(int _polygon, const vec3 &_pos);
	string name(){	return "ModelSplitPolygon";	}

	void *compose(Data *d);
private:
	int polygon;
	const vec3 &pos;
};

#endif /* ACTIONMODELSPLITPOLYGON_H_ */
