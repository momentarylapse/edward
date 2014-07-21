/*
 * ActionModelMoveBones.h
 *
 *  Created on: 18.03.2012
 *      Author: michi
 */

#ifndef ACTIONMODELMOVEBONES_H_
#define ACTIONMODELMOVEBONES_H_

#include "../../ActionMultiView.h"
class DataModel;
class vector;

class ActionModelMoveBones: public ActionMultiView
{
public:
	ActionModelMoveBones(DataModel *d);
	virtual ~ActionModelMoveBones();
	string name(){	return "ModelMoveBones";	}

	void *execute(Data *d);
	void undo(Data *d);
};

#endif /* ACTIONMODELMOVEBONES_H_ */
