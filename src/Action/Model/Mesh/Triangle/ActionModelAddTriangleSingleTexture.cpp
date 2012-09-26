/*
 * ActionModelAddTriangleSingleTexture.cpp
 *
 *  Created on: 01.04.2012
 *      Author: michi
 */

#include "ActionModelAddTriangleSingleTexture.h"
#include "ActionModelAddTriangle.h"
#include "../../../../Data/Model/DataModel.h"

ActionModelAddTriangleSingleTexture::ActionModelAddTriangleSingleTexture(DataModel *m, Array<int> &_v, int _material, Array<vector> &_sv)
{
	Array<vector> sv;
	for (int l=0;l<MODEL_MAX_TEXTURES;l++)
		for (int k=0;k<_v.num;k++)
			sv.add(_sv[k]);

	dummy = AddSubAction(new ActionModelAddTriangle(m, _v, _material, sv), m);
}

ActionModelAddTriangleSingleTexture::~ActionModelAddTriangleSingleTexture()
{
}


void *ActionModelAddTriangleSingleTexture::compose(Data *d)
{
	return dummy;
}
