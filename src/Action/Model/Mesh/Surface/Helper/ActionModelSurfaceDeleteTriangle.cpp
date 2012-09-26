/*
 * ActionModelSurfaceDeleteTriangle.cpp
 *
 *  Created on: 13.03.2012
 *      Author: michi
 */

#include "ActionModelSurfaceDeleteTriangle.h"
#include "../../../../../Data/Model/DataModel.h"

// might create an empty surface -> don't use alone!

ActionModelSurfaceDeleteTriangle::ActionModelSurfaceDeleteTriangle(int _surface, int _index)
{
	surface = _surface;
	index = _index;
}

ActionModelSurfaceDeleteTriangle::~ActionModelSurfaceDeleteTriangle()
{
}

void ActionModelSurfaceDeleteTriangle::undo(Data *d)
{
#if 0
	DataModel *m = dynamic_cast<DataModel*>(d);

	ModelSurface &s = m->Surface[surface];

	// add triangle
	s.AddTriangle(vertex[0], vertex[1], vertex[2], material, skin[0], skin[1], skin[2], index);
	//s.BuildFromTriangles();
#endif
}



void *ActionModelSurfaceDeleteTriangle::execute(Data *d)
{
#if 0
	DataModel *m = dynamic_cast<DataModel*>(d);

	ModelSurface &s = m->Surface[surface];
	ModelPolygon &t = s.Polygon[index];

	// save old data
	material = t.Material;
	for (int k=0;k<t.Side.num;k++)
		vertex[k] = t.Side[k].Vertex;
	for (int l=0;l<m->Material[material].NumTextures;l++)
		for (int k=0;k<t.Side.num;k++)
			skin[k][l] = t.Side[k].SkinVertex[l];

	// erase
	s.RemoveTriangle(index);
#endif

	return NULL;
}


