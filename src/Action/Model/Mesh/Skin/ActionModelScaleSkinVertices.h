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
	ActionModelScaleSkinVertices(DataModel *d, const vector &_param, const vector &_pos0);
	virtual ~ActionModelScaleSkinVertices();
	string name(){	return "ModelScaleSkinVertices";	}

	void *execute(Data *d);
	void undo(Data *d);

private:
	int material, texture_level;
	Array<int> surface;
	Array<int> tria;
};

#endif /* ACTIONMODELSCALESKINVERTICES_H_ */
