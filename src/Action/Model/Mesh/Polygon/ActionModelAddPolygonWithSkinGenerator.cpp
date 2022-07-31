/*
 * ActionModelAddPolygonWithSkinGenerator.cpp
 *
 *  Created on: 21.08.2012
 *      Author: michi
 */

#include "ActionModelAddPolygonWithSkinGenerator.h"
#include "ActionModelAddPolygon.h"
#include "../../../../Data/Model/DataModel.h"
#include "../../../../Data/Model/ModelMesh.h"
#include "../../../../Data/Model/SkinGenerator.h"


ActionModelAddPolygonWithSkinGenerator::ActionModelAddPolygonWithSkinGenerator(Array<int> &_v, int _material, const SkinGeneratorMulti &_sg) :
	v(_v), sg(_sg)
{
	material = _material;
}


void *ActionModelAddPolygonWithSkinGenerator::compose(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);

	Array<vec3> sv;
	for (int l=0;l<MATERIAL_MAX_TEXTURES;l++)
		for (int k=0;k<v.num;k++)
			sv.add(sg.get(m->edit_mesh->vertex[v[k]].pos, l));

	return addSubAction(new ActionModelAddPolygon(v, material, sv), m);
}
