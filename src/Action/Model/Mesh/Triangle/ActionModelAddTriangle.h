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
	ActionModelAddTriangle(DataModel *m, int _a, int _b, int _c, int _material, const vector *_sva, const vector *_svb, const vector *_svc);
	virtual ~ActionModelAddTriangle();
	string name(){	return "ModelAddTriangle";	}

	void *compose(Data *d);

private:
	int surf_no; // needed for execute_return()
};

#endif /* ACTIONMODELADDTRIANGLE_H_ */
