/*
 * ActionModelAddDodecahedron.h
 *
 *  Created on: 09.01.2013
 *      Author: michi
 */

#ifndef ACTIONMODELADDDODECAHEDRON_H_
#define ACTIONMODELADDDODECAHEDRON_H_

#include "../../../ActionGroup.h"
#include "../../../../lib/types/vector.h"
class DataModel;

class ActionModelAddDodecahedron: public ActionGroup
{
public:
	ActionModelAddDodecahedron(const vector &pos, float radius);
	virtual ~ActionModelAddDodecahedron(){}
	string name(){	return "ModelAddDodecahedron";	}

	void *compose(Data *d);
private:
	void add5(int nv, int v0, int v1, int v2, int v3, int v4, DataModel *m);
	vector pos;
	float radius;
};

#endif /* ACTIONMODELADDDODECAHEDRON_H_ */
