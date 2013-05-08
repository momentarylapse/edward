/*
 * ActionModelAddPlatonic.h
 *
 *  Created on: 09.01.2013
 *      Author: michi
 */

#ifndef ACTIONMODELADDPLATONIC_H_
#define ACTIONMODELADDPLATONIC_H_

#include "../../../ActionGroup.h"
#include "../../../../lib/math/vector.h"
class DataModel;

class ActionModelAddPlatonic: public ActionGroup
{
public:
	ActionModelAddPlatonic(const vector &pos, float radius, int type);
	virtual ~ActionModelAddPlatonic(){}
	string name(){	return "ModelAddPlatonic";	}

	void *compose(Data *d);
private:
	void AddTetrahedron(DataModel *m);
	void AddOctahedron(DataModel *m);
	void AddDodecahedron(DataModel *m);
	void AddIcosahedron(DataModel *m);
	void add5(int nv, int v0, int v1, int v2, int v3, int v4, DataModel *m);
	void add4(int nv, int v0, int v1, int v2, int v3, DataModel *m);
	void add3(int nv, int v0, int v1, int v2, DataModel *m);
	int type;
	vector pos;
	float radius;
};

#endif /* ACTIONMODELADDPLATONIC_H_ */
