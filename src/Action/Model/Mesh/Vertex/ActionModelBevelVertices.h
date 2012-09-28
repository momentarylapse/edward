/*
 * ActionModelBevelVertices.h
 *
 *  Created on: 23.09.2012
 *      Author: michi
 */

#ifndef ACTIONMODELBEVELVERTICES_H_
#define ACTIONMODELBEVELVERTICES_H_

#include "../../../ActionGroup.h"
class DataModel;

class ActionModelBevelVertices: public ActionGroup
{
public:
	ActionModelBevelVertices(float _length);
	virtual ~ActionModelBevelVertices(){}
	string name(){	return "ModelBevelVertices";	}

	void *compose(Data *d);

	void BevelVertex(DataModel *m, float length, int vi);
private:
	float length;
};

#endif /* ACTIONMODELBEVELVERTICES_H_ */
