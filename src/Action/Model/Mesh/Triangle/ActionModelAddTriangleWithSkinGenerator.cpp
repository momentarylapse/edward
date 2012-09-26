/*
 * ActionModelAddTriangleWithSkinGenerator.cpp
 *
 *  Created on: 21.08.2012
 *      Author: michi
 */

#include "ActionModelAddTriangleWithSkinGenerator.h"
#include "ActionModelAddTriangle.h"
#include "../../../../Data/Model/DataModel.h"
#include "../../../../Data/Model/SkinGenerator.h"

ActionModelAddTriangleWithSkinGenerator::ActionModelAddTriangleWithSkinGenerator(DataModel *m, Array<int> &_v, int _material, const SkinGenerator &sg)
{
	Array<vector> sv;
	for (int l=0;l<MODEL_MAX_TEXTURES;l++)
		for (int k=0;k<_v.num;k++)
			sv.add(sg.get(m->Vertex[_v[k]].pos));

	dummy = AddSubAction(new ActionModelAddTriangle(m, _v, _material, sv), m);
}

ActionModelAddTriangleWithSkinGenerator::~ActionModelAddTriangleWithSkinGenerator()
{
}


void *ActionModelAddTriangleWithSkinGenerator::compose(Data *d)
{
	return dummy;
}
