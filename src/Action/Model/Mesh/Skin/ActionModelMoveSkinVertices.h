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
	ActionModelMoveSkinVertices(DataModel *d, const vector &param, const vector &pos0, int texture_level);
	virtual ~ActionModelMoveSkinVertices();
	virtual string name(){	return "ModelMoveSkinVertices";	}
	virtual string message(){	return "SkinChange";	}

	virtual void *execute(Data *d);
	virtual void undo(Data *d);

private:
	int texture_level;
	Array<int> surface;
	Array<int> tria;
};

#endif /* ACTIONMODELMOVESKINVERTICES_H_ */
