/*
 * ActionModelMirrorSkinVertices.h
 *
 *  Created on: 15.03.2012
 *      Author: michi
 */

#ifndef ACTIONMODELMIRRORSKINVERTICES_H_
#define ACTIONMODELMIRRORSKINVERTICES_H_

#include "../../../ActionMultiView.h"
class vector;
class DataModel;

class ActionModelMirrorSkinVertices: public ActionMultiView
{
public:
	ActionModelMirrorSkinVertices(DataModel *d, const vector &param, const vector &pos0, int texture_level);
	virtual ~ActionModelMirrorSkinVertices();
	string name(){	return "ModelMirrorSkinVertices";	}

	void *execute(Data *d);
	void undo(Data *d);

private:
	int texture_level;
	Array<int> surface;
	Array<int> tria;
};

#endif /* ACTIONMODELMIRRORSKINVERTICES_H_ */
