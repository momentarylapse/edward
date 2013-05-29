/*
 * ActionModelScaleSkinVertices.h
 *
 *  Created on: 15.03.2012
 *      Author: michi
 */

#ifndef ACTIONMODELSCALESKINVERTICES_H_
#define ACTIONMODELSCALESKINVERTICES_H_

#include "../../../ActionMultiView.h"
class vector;
class DataModel;

class ActionModelScaleSkinVertices: public ActionMultiView
{
public:
	ActionModelScaleSkinVertices(DataModel *d, const vector &param, const vector &pos0, int texture_level);
	virtual ~ActionModelScaleSkinVertices();
	string name(){	return "ModelScaleSkinVertices";	}

	void *execute(Data *d);
	void undo(Data *d);

private:
	int texture_level;
	Array<int> surface;
	Array<int> tria;
};

#endif /* ACTIONMODELSCALESKINVERTICES_H_ */
