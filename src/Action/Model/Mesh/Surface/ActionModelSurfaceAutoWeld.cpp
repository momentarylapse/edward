/*
 * ActionModelSurfaceAutoWeld.cpp
 *
 *  Created on: 03.06.2012
 *      Author: michi
 */

#include "ActionModelSurfaceAutoWeld.h"
#include "Helper/ActionModelJoinSurfaces.h"
#include "../Vertex/Helper/ActionModelDeleteUnusedVertex.h"
#include "Helper/ActionModelSurfaceRelinkPolygon.h"
#include "../../../../Data/Model/ModelSurface.h"

ActionModelSurfaceAutoWeld::ActionModelSurfaceAutoWeld(int _surface1, int _surface2, float _epsilon)
{
	surface1 = _surface1;
	surface2 = _surface2;
	epsilon = _epsilon;
}


void *ActionModelSurfaceAutoWeld::compose(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);

	if (surface1 >= surface2)
		throw ActionException("s1 >= s2");

	ModelSurface *a = &m->Surface[surface1];
	ModelSurface *b = &m->Surface[surface2];

	if (a >= b)
		throw ActionException("a >= b... array reference broken");

	if (!a->TestSanity("AutoWeld prae a"))
		throw ActionException("s1 evil pre");
	if (!b->TestSanity("AutoWeld prae b"))
		throw ActionException("s2 evil pre");


	// find pairs of vertices close to each other
	// use edges instead???
	Array<int> wa, wb;
	foreach(int va, a->Vertex){
		foreach(int vb, b->Vertex){
			if ((m->Vertex[va].pos - m->Vertex[vb].pos).length() <= epsilon){
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
	a = (ModelSurface*)AddSubAction(new ActionModelJoinSurfaces(surface1, surface2), m);

	// relink triangles
	foreachib(ModelPolygon &t, a->Polygon, ti){
		Array<int> v;
		v.resize(t.Side.num);
		bool relink = false;
		for (int k=0;k<t.Side.num;k++){
			v[k] = t.Side[k].Vertex;
			foreachi(int w, wb, i){
				if (t.Side[k].Vertex == w){
					relink = true;
					v[k] = wa[i];
				}
			}
		}
		if (relink){
			AddSubAction(new ActionModelSurfaceRelinkPolygon(surface1, ti, v), m);
			_foreach_it_.update(); // TODO
		}
	}
	if (!a->TestSanity("AutoWeld relink tria"))
		throw ActionException("s1 evil post");

	// remove obsolete vertices
	Set<int> vv;
	foreach(int w, wb)
		vv.add(w);
	foreachb(int ww, vv){
		/*if (m->Vertex[ww].RefCount > 0)
			foreach*/
		//a->Vertex.erase(ww);
		//m->Vertex[ww].Surface = -1;
		AddSubAction(new ActionModelDeleteUnusedVertex(ww), m);
	}

	return a;
}
