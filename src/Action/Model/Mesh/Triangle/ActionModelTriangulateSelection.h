/*
 * ActionModelTriangulateSelection.h
 *
 *  Created on: 26.09.2012
 *      Author: michi
 */

#ifndef ACTIONMODELTRIANGULATESELECTION_H_
#define ACTIONMODELTRIANGULATESELECTION_H_

#include "../../../ActionGroup.h"
class DataModel;
class vector;

class ActionModelTriangulateSelection: public ActionGroup
{
public:
	ActionModelTriangulateSelection();
	virtual ~ActionModelTriangulateSelection(){}
	string name(){	return "ModelTriangulateSelection";	}

	void *compose(Data *d);
};

#endif /* ACTIONMODELTRIANGULATESELECTION_H_ */
