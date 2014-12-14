/*
 * ActionModelTransformSkinVertices.h
 *
 *  Created on: 15.03.2012
 *      Author: michi
 */

#ifndef ACTIONMODELTRANSFORMSKINVERTICES_H_
#define ACTIONMODELTRANSFORMSKINVERTICES_H_

#include "../../../ActionMultiView.h"
class vector;
class DataModel;

class ActionModelTransformSkinVertices: public ActionMultiView
{
public:
	ActionModelTransformSkinVertices(DataModel *d, int texture_level);
	virtual ~ActionModelTransformSkinVertices();
	virtual string name(){	return "ModelTransformSkinVertices";	}
	virtual const string &message();

	virtual void *execute(Data *d);
	virtual void undo(Data *d);

private:
	int texture_level;
	Array<int> surface;
	Array<int> tria;
};

#endif /* ACTIONMODELTRANSFORMSKINVERTICES_H_ */
