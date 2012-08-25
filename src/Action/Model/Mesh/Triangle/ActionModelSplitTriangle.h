/*
 * ActionModelSplitTriangle.h
 *
 *  Created on: 01.04.2012
 *      Author: michi
 */

#ifndef ACTIONMODELSPLITTRIANGLE_H_
#define ACTIONMODELSPLITTRIANGLE_H_

#include "../../../ActionGroup.h"
class DataModel;
class vector;

class ActionModelSplitTriangle: public ActionGroup
{
public:
	ActionModelSplitTriangle(DataModel *m, int _surface, int _triangle, const vector &_pos);
	virtual ~ActionModelSplitTriangle();
	string name(){	return "ModelSplitTriangle";	}
};

#endif /* ACTIONMODELSPLITTRIANGLE_H_ */
