/*
 * ActionModel__JoinSurfaces.cpp
 *
 *  Created on: 13.03.2012
 *      Author: michi
 */

#include "ActionModel__JoinSurfaces.h"
#include "../../Data/Model/DataModel.h"
#include <assert.h>


// creates a "disjoint" surface -> don't use alone!

ActionModel__JoinSurfaces::ActionModel__JoinSurfaces(int _surface1, int _surface2)
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

ActionModel__JoinSurfaces::~ActionModel__JoinSurfaces()
{
}



void *ActionModel__JoinSurfaces::execute(Data *d)
{
	msg_db_r("SurfJoin", 1);
	msg_write("__join surf do");
	DataModel *m = dynamic_cast<DataModel*>(d);

	assert(surface1 < m->Surface.num);
	assert(surface2 < m->Surface.num);

	ModeModelSurface *a = &m->Surface[surface1];
	ModeModelSurface *b = &m->Surface[surface2];

	// save old data
	old_edges1 = a->Edge.num;
	old_vertices1 = a->Vertex.num;
	old_trias1 = a->Triangle.num;

	a->TestSanity("Join prae a");
	b->TestSanity("Join prae b");

	// correct edge data of b
	foreach(b->Edge, e){
		if (e.Triangle[0] >= 0)
			e.Triangle[0] += a->Triangle.num;
		if (e.Triangle[1] >= 0)
			e.Triangle[1] += a->Triangle.num;
	}

	// correct triangle data of b
	foreach(b->Triangle, t)
		for (int k=0;k<3;k++)
			t.Edge[k] += a->Edge.num;

	// correct vertex data of b
	foreach(b->Vertex, v)
		m->Vertex[v].Surface = surface1;

	// insert data
	a->Vertex.join(b->Vertex);
	a->Edge.append(b->Edge);
	a->Triangle.append(b->Triangle);

	// remove surface
	m->Surface.erase(surface2);
	a = &m->Surface[surface1];
	a->TestSanity("Join post a");

	// correct vertices of surfaces above b
	foreach(m->Vertex, v)
		if (v.Surface >= surface2)
			v.Surface --;

	msg_db_l(1);
	return a;
}



void ActionModel__JoinSurfaces::redo(Data *d)
{	execute(d);	}



void ActionModel__JoinSurfaces::undo(Data *d)
{
	msg_write("__join surf undo");
	DataModel *m = dynamic_cast<DataModel*>(d);

	ModeModelSurface *b = m->AddSurface(surface2);
	ModeModelSurface *a = &m->Surface[surface1];

	// move triangles
	b->Triangle.append(a->Triangle.sub(old_trias1, -1));
	a->Triangle.resize(old_trias1);

	// rebuild
	a->BuildFromTriangles();
	b->BuildFromTriangles();
}
