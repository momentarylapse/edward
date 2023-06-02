/*
 * ActionModelAddPolygonSingleTexture.cpp
 *
 *  Created on: 01.04.2012
 *      Author: michi
 */

#include "ActionModelAddPolygonSingleTexture.h"
#include "ActionModelAddPolygon.h"
#include "../../../../data/model/DataModel.h"



ActionModelAddPolygonSingleTexture::ActionModelAddPolygonSingleTexture(const Array<int> &_v, int _material, const Array<vec3> &_sv)
{
	material = _material;
	v = _v;
	sv = _sv;
}


void *ActionModelAddPolygonSingleTexture::compose(Data *d) {
	DataModel *m = dynamic_cast<DataModel*>(d);

	Array<vec3> sv2;
	for (int l=0;l<MATERIAL_MAX_TEXTURES;l++)
		for (int k=0;k<v.num;k++)
			sv2.add(sv[k]);

	return addSubAction(new ActionModelAddPolygon(v, material, sv2), m);
}
