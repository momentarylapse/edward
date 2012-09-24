/*
 * ActionModelMoveSkinVertices.h
 *
 *  Created on: 15.03.2012
 *      Author: michi
 */

#ifndef ACTIONMODELMOVESKINVERTICES_H_
#define ACTIONMODELMOVESKINVERTICES_H_

#include "../../../ActionMultiView.h"
class vector;
class DataModel;

class ActionModelMoveSkinVertices: public ActionMultiView
{
public:
	ActionModelMoveSkinVertices(DataModel *d, const vector &_param, const vector &_pos0);
	virtual ~ActionModelMoveSkinVertices();
	string name(){	return "ModelMoveSkinVertices";	}

	void *execute(Data *d);
	void undo(Data *d);

private:
	int material, texture_level;
	Array<int> surface;
	Array<int> tria;
};

#endif /* ACTIONMODELMOVESKINVERTICES_H_ */
