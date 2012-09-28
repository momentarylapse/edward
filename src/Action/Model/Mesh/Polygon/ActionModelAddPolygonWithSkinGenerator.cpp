/*
 * ActionModelAddPolygonWithSkinGenerator.cpp
 *
 *  Created on: 21.08.2012
 *      Author: michi
 */

#include "ActionModelAddPolygonWithSkinGenerator.h"
#include "ActionModelAddPolygon.h"
#include "../../../../Data/Model/DataModel.h"
#include "../../../../Data/Model/SkinGenerator.h"


ActionModelAddPolygonWithSkinGenerator::ActionModelAddPolygonWithSkinGenerator(Array<int> &_v, int _material, const SkinGenerator &_sg) :
	v(_v), sg(_sg)
{
	material = _material;
}


void *ActionModelAddPolygonWithSkinGenerator::compose(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);

	Array<vector> sv;
	for (int l=0;l<MODEL_MAX_TEXTURES;l++)
		for (int k=0;k<v.num;k++)
			sv.add(sg.get(m->Vertex[v[k]].pos));

	return AddSubAction(new ActionModelAddPolygon(v, material, sv), m);
}