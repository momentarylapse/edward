/*
 * ActionModelTriangulateVertices.h
 *
 *  Created on: Jan 23, 2014
 *      Author: ankele
 */

#ifndef ACTIONMODELTRIANGULATEVERTICES_H_
#define ACTIONMODELTRIANGULATEVERTICES_H_

#include "../../../ActionGroup.h"
#include "../../../../lib/base/set.h"

class ActionModelTriangulateVertices: public ActionGroup {
public:
	ActionModelTriangulateVertices();
	virtual ~ActionModelTriangulateVertices(){}
	string name(){	return "ModelTriangulateVertices";	}

	void *compose(Data *d);
	void add_tria(Data*d, int i0, int i1, int i2, base::set<int> &used, Array<int> &boundary);
	void add_tria2(Data*d, int i0, int i1, int i2, Array<int> &boundary);
};

#endif /* ACTIONMODELTRIANGULATEVERTICES_H_ */
