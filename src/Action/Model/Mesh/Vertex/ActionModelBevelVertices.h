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
class ModelVertex;

class ActionModelBevelVertices: public ActionGroup
{
public:
	ActionModelBevelVertices(DataModel *m, float length);
	virtual ~ActionModelBevelVertices();
	string name(){	return "ModelBevelVertices";	}

	void BevelVertex(DataModel *m, float length, int vi);
};

#endif /* ACTIONMODELBEVELVERTICES_H_ */
