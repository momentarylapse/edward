/*
 * ActionModelAddPolygonSingleTexture.h
 *
 *  Created on: 01.04.2012
 *      Author: michi
 */

#ifndef ACTIONMODELADDPOLYGONSINGLETEXTURE_H_
#define ACTIONMODELADDPOLYGONSINGLETEXTURE_H_

#include "../../../ActionGroup.h"
class DataModel;
class vector;

class ActionModelAddPolygonSingleTexture: public ActionGroup
{
public:
	ActionModelAddPolygonSingleTexture(Array<int> &_v, int _material, Array<vector> &_sv);
	virtual ~ActionModelAddPolygonSingleTexture(){}
	string name(){	return "ModelAddPolygonSingleTexture";	}

	void *compose(Data *d);

private:
	Array<int> &v;
	int material;
	Array<vector> &sv;
};

#endif /* ACTIONMODELADDPOLYGONSINGLETEXTURE_H_ */
