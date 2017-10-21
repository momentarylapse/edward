/*
 * ActionModelJoinSurfaces.cpp
 *
 *  Created on: 13.03.2012
 *      Author: michi
 */

#include "ActionModelJoinSurfaces.h"
#include "../../../../../Data/Model/DataModel.h"
#include <assert.h>


// creates a "disjoint" surface -> don't use alone!

ActionModelJoinSurfaces::ActionModelJoinSurfaces(int _surface1, int _surface2)
{
	assert(_surface1 >= 0);
	assert(_surface2 >= 0);
	assert(_surface1 != _surface2);
	if (_surface1 > _surface2){
		surface1 = _surface2;
		surface2 = _surface1;
	}else{
		surface1 = _surface1;
		surface2 = _surface2;
	}
}

ActionModelJoinSurfaces::~ActionModelJoinSurfaces()
{
}



void *ActionModelJoinSurfaces::execute(Data *d)
{
	msg_db_f("SurfJoin", 1);
	DataModel *m = dynamic_cast<DataModel*>(d);

	assert(surface1 < m->surface.num);
	assert(surface2 < m->surface.num);

	ModelSurface *a = &m->surface[surface1];
	ModelSurface *b = &m->surface[surface2];

	// save old data
	old_edges1 = a->edge.num;
	old_vertices1 = a->vertex.num;
	old_trias1 = a->polygon.num;

	a->testSanity("Join prae a");
	b->testSanity("Join prae b");

	// correct edge data of b
	for (ModelEdge &e: b->edge){
		if (e.polygon[0] >= 0)
			e.polygon[0] += a->polygon.num;
		if (e.polygon[1] >= 0)
			e.polygon[1] += a->polygon.num;
	}

	// correct triangle data of b
	for (ModelPolygon &t: b->polygon)
		for (int k=0;k<t.side.num;k++)
			t.side[k].edge += a->edge.num;

	// correct vertex data of b
	for (int v: b->vertex)
		m->vertex[v].surface = surface1;

	// insert data
	a->vertex.join(b->vertex);
	a->edge.append(b->edge);
	a->polygon.append(b->polygon);

	// remove surface
	m->surface.erase(surface2);
	a = &m->surface[surface1];
	a->testSanity("Join post a");

	// correct vertices of surfaces above b
	for (ModelVertex &v: m->vertex)
		if (v.surface >= surface2)
			v.surface --;
	return a;
}



void ActionModelJoinSurfaces::undo(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);

	ModelSurface *b = m->addSurface(surface2);
	ModelSurface *a = &m->surface[surface1];

	// move triangles
	b->polygon.append(a->polygon.sub(old_trias1, -1));
	a->polygon.resize(old_trias1);

	// rebuild
	a->buildFromPolygons();
	b->buildFromPolygons();
}
