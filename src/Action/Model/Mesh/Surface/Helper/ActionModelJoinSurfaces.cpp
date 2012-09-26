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
	msg_db_r("SurfJoin", 1);
	DataModel *m = dynamic_cast<DataModel*>(d);

	assert(surface1 < m->Surface.num);
	assert(surface2 < m->Surface.num);

	ModelSurface *a = &m->Surface[surface1];
	ModelSurface *b = &m->Surface[surface2];

	// save old data
	old_edges1 = a->Edge.num;
	old_vertices1 = a->Vertex.num;
	old_trias1 = a->Polygon.num;

	a->TestSanity("Join prae a");
	b->TestSanity("Join prae b");

	// correct edge data of b
	foreach(ModelEdge &e, b->Edge){
		if (e.Polygon[0] >= 0)
			e.Polygon[0] += a->Polygon.num;
		if (e.Polygon[1] >= 0)
			e.Polygon[1] += a->Polygon.num;
	}

	// correct triangle data of b
	foreach(ModelPolygon &t, b->Polygon)
		for (int k=0;k<t.Side.num;k++)
			t.Side[k].Edge += a->Edge.num;

	// correct vertex data of b
	foreach(int v, b->Vertex)
		m->Vertex[v].Surface = surface1;

	// insert data
	a->Vertex.join(b->Vertex);
	a->Edge.append(b->Edge);
	a->Polygon.append(b->Polygon);

	// remove surface
	m->Surface.erase(surface2);
	a = &m->Surface[surface1];
	a->TestSanity("Join post a");

	// correct vertices of surfaces above b
	foreach(ModelVertex &v, m->Vertex)
		if (v.Surface >= surface2)
			v.Surface --;

	msg_db_l(1);
	return a;
}



void ActionModelJoinSurfaces::undo(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);

	ModelSurface *b = m->AddSurface(surface2);
	ModelSurface *a = &m->Surface[surface1];

	// move triangles
	b->Polygon.append(a->Polygon.sub(old_trias1, -1));
	a->Polygon.resize(old_trias1);

	// rebuild
	a->BuildFromPolygons();
	b->BuildFromPolygons();
}
