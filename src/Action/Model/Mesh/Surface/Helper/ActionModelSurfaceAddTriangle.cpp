/*
 * ActionModelSurfaceAddTriangle.cpp
 *
 *  Created on: 13.03.2012
 *      Author: michi
 */

#include "ActionModelSurfaceAddTriangle.h"
#include "../../../../../Data/Model/DataModel.h"

// might create a "disjoint" surface -> don't use alone!

ActionModelSurfaceAddTriangle::ActionModelSurfaceAddTriangle(int _surface, int _a, int _b, int _c, int _material, const vector *_sva, const vector *_svb, const vector *_svc, int _index)
{
	surface = _surface;
	index = _index;
	a = _a;
	b = _b;
	c = _c;
	material = _material;
	for (int l=0;l<MODEL_MAX_TEXTURES;l++){
		sv[0][l] = _sva[l];
		sv[1][l] = _svb[l];
		sv[2][l] = _svc[l];
	}
}

ActionModelSurfaceAddTriangle::~ActionModelSurfaceAddTriangle()
{
}

void ActionModelSurfaceAddTriangle::undo(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);

	ModelSurface &s = m->Surface[surface];

	if (index >= 0)
		s.RemoveTriangle(index);
	else
		s.RemoveTriangle(s.Triangle.num -1);
}



void *ActionModelSurfaceAddTriangle::execute(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);

	ModelSurface &s = m->Surface[surface];

	// add triangle
	s.AddTriangle(a, b, c, material, sv[0], sv[1], sv[2], index);

	if (index >= 0)
		return &s.Triangle[index];
	return &s.Triangle.back();
}


