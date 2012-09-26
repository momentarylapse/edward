/*
 * ActionAddTriangle.h
 *
 *  Created on: 05.03.2012
 *      Author: michi
 */

#ifndef ACTIONMODELADDTRIANGLE_H_
#define ACTIONMODELADDTRIANGLE_H_

#include "../../../ActionGroup.h"
class DataModel;
class vector;

class ActionModelAddTriangle: public ActionGroup
{
public:
	ActionModelAddTriangle(DataModel *m, Array<int> &_v, int _material, Array<vector> &_sv);
	virtual ~ActionModelAddTriangle();
	string name(){	return "ModelAddTriangle";	}

	void *compose(Data *d);

private:
	int surf_no; // needed for execute_return()
};

#endif /* ACTIONMODELADDTRIANGLE_H_ */
