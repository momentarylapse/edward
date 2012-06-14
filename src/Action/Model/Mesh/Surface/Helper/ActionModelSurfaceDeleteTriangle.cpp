/*
 * ActionModelSurfaceDeleteTriangle.cpp
 *
 *  Created on: 13.03.2012
 *      Author: michi
 */

#include "ActionModelSurfaceDeleteTriangle.h"
#include "../../../../../Data/Model/DataModel.h"

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
	DataModel *m = dynamic_cast<DataModel*>(d);

	ModeModelSurface &s = m->Surface[surface];

	// add triangle
	s.AddTriangle(vertex[0], vertex[1], vertex[2], material, skin[0], skin[1], skin[2], index);
	//s.BuildFromTriangles();
}



void *ActionModelSurfaceDeleteTriangle::execute(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);

	ModeModelSurface &s = m->Surface[surface];
	ModeModelTriangle &t = s.Triangle[index];

	// save old data
	material = t.Material;
	for (int k=0;k<3;k++)
		vertex[k] = t.Vertex[k];
	for (int l=0;l<m->Material[material].NumTextures;l++)
		for (int k=0;k<3;k++)
			skin[k][l] = t.SkinVertex[l][k];

	// erase
	s.RemoveTriangle(index);

	return NULL;
}


