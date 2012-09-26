/*
 * ActionModelSurfaceAutoWeld.cpp
 *
 *  Created on: 03.06.2012
 *      Author: michi
 */

#include "ActionModelSurfaceAutoWeld.h"
#include "Helper/ActionModelJoinSurfaces.h"
#include "../Vertex/Helper/ActionModelDeleteUnusedVertex.h"
#include "Helper/ActionModelSurfaceRelinkTriangle.h"
#include "../../../../Data/Model/ModeModelSurface.h"

ActionModelSurfaceAutoWeld::ActionModelSurfaceAutoWeld(DataModel *m, int _surface1, int _surface2, float _epsilon, bool _ignore_failure)
{
	surface1 = _surface1;
	surface2 = _surface2;
	epsilon = _epsilon;
	ignore_failure = _ignore_failure;
}


void *ActionModelSurfaceAutoWeld::compose(Data *d)
{
	msg_db_r("SurfWeld", 1);
	DataModel *m = dynamic_cast<DataModel*>(d);

	if (surface1 >= surface2)
		throw ActionException("SurfaceWeld: s1 >= s2");

	ModelSurface *a = &m->Surface[surface1];
	ModelSurface *b = &m->Surface[surface2];

	if (a >= b)
		throw ActionException("SurfaceWeld: a >= b... array reference broken");

	a->TestSanity("AutoWeld prae a");
	b->TestSanity("AutoWeld prae b");


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

	if ((wa.num == 0) && (!ignore_failure))
		throw ActionException("SurfaceWeld: nothing to weld");

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
			AddSubAction(new ActionModelSurfaceRelinkTriangle(m, surface1, ti, v), m);
			_foreach_it_.update(); // TODO
		}
	}
	a->TestSanity("AutoWeld relink tria");

	// remove obsolete vertices
	Set<int> vv;
	foreach(int w, wb)
		vv.add(w);
	foreachb(int ww, vv){
		a->Vertex.erase(ww);
		m->Vertex[ww].Surface = -1;
		AddSubAction(new ActionModelDeleteUnusedVertex(ww), m);
	}

	msg_db_l(1);
	return a;
}
