/*
 * ActionModelSurfaceAutoWeld.cpp
 *
 *  Created on: 03.06.2012
 *      Author: michi
 */

#include "ActionModelSurfaceAutoWeld.h"
#include "helper/ActionModelJoinSurfaces.h"
#include "../vertex/helper/ActionModelDeleteUnusedVertex.h"
#include "helper/ActionModelSurfaceRelinkPolygon.h"
#include "../../../../lib/os/msg.h"

ActionModelSurfaceAutoWeld::ActionModelSurfaceAutoWeld(int _surface1, int _surface2, float _epsilon)
{
	surface1 = _surface1;
	surface2 = _surface2;
	epsilon = _epsilon;
}


void *ActionModelSurfaceAutoWeld::compose(Data *d)
{
	msg_todo("ActionModelSurfaceAutoWeld");
#if 0
	DataModel *m = dynamic_cast<DataModel*>(d);

	if (surface1 >= surface2)
		throw ActionException("s1 >= s2");

	ModelSurface *a = &m->surface[surface1];
	ModelSurface *b = &m->surface[surface2];

	if (a >= b)
		throw ActionException("a >= b... array reference broken");

	if (!a->testSanity("AutoWeld prae a"))
		throw ActionException("s1 evil pre");
	if (!b->testSanity("AutoWeld prae b"))
		throw ActionException("s2 evil pre");


	// find pairs of vertices close to each other
	// use edges instead???
	Array<int> wa, wb;
	for (int va: a->vertex){
		for (int vb: b->vertex){
			if ((m->vertex[va].pos - m->vertex[vb].pos).length() <= epsilon){
				wa.add(va);
				wb.add(vb);
				break;
			}
		}
	}

	// nothing to weld
	if (wa.num == 0)
		return NULL;

	// join
	a = (ModelSurface*)addSubAction(new ActionModelJoinSurfaces(surface1, surface2), m);

	// relink triangles
	foreachib(ModelPolygon &t, a->polygon, ti){
		Array<int> v;
		v.resize(t.side.num);
		bool relink = false;
		for (int k=0;k<t.side.num;k++){
			v[k] = t.side[k].vertex;
			foreachi(int w, wb, i){
				if (t.side[k].vertex == w){
					relink = true;
					v[k] = wa[i];
				}
			}
		}
		if (relink){
			addSubAction(new ActionModelSurfaceRelinkPolygon(surface1, ti, v), m);
			_foreach_it_.update(); // TODO
		}
	}
	if (!a->testSanity("AutoWeld relink tria"))
		throw ActionException("s1 evil post");

	// remove obsolete vertices
	base::set<int> vv;
	for (int w: wb)
		vv.add(w);
	foreachb(int ww, vv){
		/*if (m->Vertex[ww].RefCount > 0)
			foreach*/
		//a->Vertex.erase(ww);
		//m->Vertex[ww].Surface = -1;
		addSubAction(new ActionModelDeleteUnusedVertex(ww), m);
	}

	return a;
#endif
	return NULL;
}
