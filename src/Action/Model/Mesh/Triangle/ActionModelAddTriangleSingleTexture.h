/*
 * ActionModelAddTriangleSingleTexture.h
 *
 *  Created on: 01.04.2012
 *      Author: michi
 */

#ifndef ACTIONMODELADDTRIANGLESINGLETEXTURE_H_
#define ACTIONMODELADDTRIANGLESINGLETEXTURE_H_

#include "../../../ActionGroup.h"
class DataModel;
class vector;

class ActionModelAddTriangleSingleTexture: public ActionGroup
{
public:
	ActionModelAddTriangleSingleTexture(DataModel *m, Array<int> &_v, int _material, Array<vector> &_sv);
	virtual ~ActionModelAddTriangleSingleTexture();
	string name(){	return "ModelAddTriangleSingleTexture";	}

	void *compose(Data *d);

private:
	void *dummy;
};

#endif /* ACTIONMODELADDTRIANGLESINGLETEXTURE_H_ */
