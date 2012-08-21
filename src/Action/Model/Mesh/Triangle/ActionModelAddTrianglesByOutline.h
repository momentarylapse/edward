/*
 * ActionModelAddTrianglesByOutline.h
 *
 *  Created on: 07.03.2012
 *      Author: michi
 */

#ifndef ACTIONMODELADDTRIANGLESBYOUTLINE_H_
#define ACTIONMODELADDTRIANGLESBYOUTLINE_H_

#include "../../../Action.h"
#include "../../../ActionGroup.h"
#include "../../../../lib/types/types.h"
class DataModel;
class SkinGenerator;

class ActionModelAddTrianglesByOutline: public ActionGroup
{
public:
	ActionModelAddTrianglesByOutline(Array<int> vertex, DataModel *data);
	virtual ~ActionModelAddTrianglesByOutline();
	string name(){	return "ModelAddTrianglesByOutline";	}

private:
	void CreateTrianglesFlat(DataModel *m, Array<int> &v, const SkinGenerator &sg);
};

#endif /* ACTIONMODELADDTRIANGLESBYOUTLINE_H_ */
