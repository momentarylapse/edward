/*
 * ActionModelAddPolygonWithSkinGenerator.h
 *
 *  Created on: 21.08.2012
 *      Author: michi
 */

#ifndef ACTIONMODELADDPOLYGONWITHSKINGENERATOR_H_
#define ACTIONMODELADDPOLYGONWITHSKINGENERATOR_H_

#include "../../../ActionGroup.h"
class DataModel;
class SkinGeneratorMulti;

class ActionModelAddPolygonWithSkinGenerator: public ActionGroup
{
public:
	ActionModelAddPolygonWithSkinGenerator(Array<int> &_v, int _material, const SkinGeneratorMulti &_sg);
	virtual ~ActionModelAddPolygonWithSkinGenerator(){}
	string name(){	return "ModelAddPolygonWithSkinGenerator";	}

	void *compose(Data *d);

private:
	Array<int> &v;
	int material;
	const SkinGeneratorMulti &sg;
};

#endif /* ACTIONMODELADDPOLYGONWITHSKINGENERATOR_H_ */
