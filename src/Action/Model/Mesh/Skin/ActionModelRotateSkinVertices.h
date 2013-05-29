/*
 * ActionModelRotateSkinVertices.h
 *
 *  Created on: 15.03.2012
 *      Author: michi
 */

#ifndef ACTIONMODELROTATESKINVERTICES_H_
#define ACTIONMODELROTATESKINVERTICES_H_

#include "../../../ActionMultiView.h"
class vector;
class DataModel;

class ActionModelRotateSkinVertices: public ActionMultiView
{
public:
	ActionModelRotateSkinVertices(DataModel *d, const vector &param, const vector &pos0, int texture_level);
	virtual ~ActionModelRotateSkinVertices();
	string name(){	return "ModelRotateSkinVertices";	}

	void *execute(Data *d);
	void undo(Data *d);

private:
	int texture_level;
	Array<int> surface;
	Array<int> tria;
};

#endif /* ACTIONMODELROTATESKINVERTICES_H_ */
