/*
 * ActionModel__SurfaceDeleteTriangle.cpp
 *
 *  Created on: 13.03.2012
 *      Author: michi
 */

#include "ActionModel__SurfaceDeleteTriangle.h"
#include "../../../../../Data/Model/DataModel.h"

ActionModel__SurfaceDeleteTriangle::ActionModel__SurfaceDeleteTriangle(int _surface, int _index)
{
	surface = _surface;
	index = _index;
}

ActionModel__SurfaceDeleteTriangle::~ActionModel__SurfaceDeleteTriangle()
{
}

void ActionModel__SurfaceDeleteTriangle::undo(Data *d)
{
	msg_write("__surf del tria undo");
	DataModel *m = dynamic_cast<DataModel*>(d);

	ModeModelSurface *s = &m->Surface[surface];

	// add triangle
	s->AddTriangle(vertex[0], vertex[1], vertex[2], material, skin[0], skin[1], skin[2], index);
	s->BuildFromTriangles();
}



void *ActionModel__SurfaceDeleteTriangle::execute(Data *d)
{
	msg_write("__surf del tria do");
	DataModel *m = dynamic_cast<DataModel*>(d);

	ModeModelSurface *s = &m->Surface[surface];
	ModeModelTriangle *t = &s->Triangle[index];

	// save old data
	material = t->Material;
	for (int k=0;k<3;k++)
		vertex[k] = t->Vertex[k];
	for (int l=0;l<m->Material[material].NumTextures;l++)
		for (int k=0;k<3;k++)
			skin[k][l] = t->SkinVertex[l][k];

	// erase
	s->Triangle.erase(index);
	s->BuildFromTriangles();
	return NULL;
}


