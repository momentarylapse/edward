/*
 * ActionModelAddTeapot.h
 *
 *  Created on: 11.01.2013
 *      Author: michi
 */

#ifndef ACTIONMODELADDTEAPOT_H_
#define ACTIONMODELADDTEAPOT_H_

#include "../../../ActionGroup.h"
#include "../../../../lib/types/vector.h"
class DataModel;

class ActionModelAddTeapot: public ActionGroup
{
public:
	ActionModelAddTeapot(const vector &pos, float radius, int samples);
	virtual ~ActionModelAddTeapot(){}
	string name(){	return "ModelAddTeapot";	}

	void *compose(Data *d);
private:
	void addBezier(int v00, int v01, int v02, int v03, int v10, int v11, int v12, int v13, int v20, int v21, int v22, int v23, int v30, int v31, int v32, int v33, DataModel *m);
	vector pos;
	float radius;
	int samples;
};

#endif /* ACTIONMODELADDTEAPOT_H_ */
