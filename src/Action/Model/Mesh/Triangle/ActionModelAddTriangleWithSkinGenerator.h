/*
 * ActionModelAddTriangleWithSkinGenerator.h
 *
 *  Created on: 21.08.2012
 *      Author: michi
 */

#ifndef ACTIONMODELADDTRIANGLEWITHSKINGENERATOR_H_
#define ACTIONMODELADDTRIANGLEWITHSKINGENERATOR_H_

#include "../../../ActionGroup.h"
class DataModel;
class SkinGenerator;

class ActionModelAddTriangleWithSkinGenerator: public ActionGroup
{
public:
	ActionModelAddTriangleWithSkinGenerator(DataModel *m, int _a, int _b, int _c, int _material, const SkinGenerator &sg);
	virtual ~ActionModelAddTriangleWithSkinGenerator();
	string name(){	return "ModelAddTriangleWithSkinGenerator";	}

	void *execute_return(Data *d);

private:
	void *dummy;
};

#endif /* ACTIONMODELADDTRIANGLEWITHSKINGENERATOR_H_ */
