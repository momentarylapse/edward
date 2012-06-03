/*
 * ActionModelSurfaceAutoWeld.cpp
 *
 *  Created on: 03.06.2012
 *      Author: michi
 */

#include "ActionModelSurfaceAutoWeld.h"
#include "ActionModel__JoinSurfaces.h"
#include "ActionModel__DeleteVertex.h"
#include "ActionModelSurfaceRelinkTriangle.h"
#include "../../Data/Model/ModeModelSurface.h"

ActionModelSurfaceAutoWeld::ActionModelSurfaceAutoWeld(DataModel *m, int _surface1, int _surface2, float d)
{
	msg_db_r("SurfWeld", 1);

	ModeModelSurface *a = &m->Surface[_surface1];
	ModeModelSurface *b = &m->Surface[_surface2];

	if (a > b)
		msg_error("SurfaceWeld: a > b... array reference broken");

	a->TestSanity("AutoWeld prae a");
	b->TestSanity("AutoWeld prae b");


	// find pairs of vertices close to each other
	// use edges instead???
	Array<int> wa, wb;
	foreach(a->Vertex, va){
		foreach(b->Vertex, vb){
			if (VecLength(m->Vertex[va].pos - m->Vertex[vb].pos) <= d){
				wa.add(va);
				wb.add(vb);
				break;
			}
		}
	}

	// join
	a = (ModeModelSurface*)AddSubAction(new ActionModel__JoinSurfaces(_surface1, _surface2), m);

	// relink triangles
	foreachbi(a->Triangle, t, ti){
		int v[3];
		bool relink = false;
		for (int k=0;k<3;k++){
			v[k] = t.Vertex[k];
			foreachi(wb, w, i){
				if (t.Vertex[k] == w){
					relink = true;
					v[k] = wa[i];
				}
			}
		}
		if (relink)
			AddSubAction(new ActionModelSurfaceRelinkTriangle(m, _surface1, ti, v[0], v[1], v[2]), m);
	}
	a->TestSanity("AutoWeld relink tria");

/*	// relink edges
	foreach(a->Edge, e){
		for (int k=0;k<2;k++){
			foreachi(wb, w, i){
				if (e.Vertex[k] == w)
					e.Vertex[k] = wa[i];
			}
		}
	}
	a->TestSanity("AutoWeld relink edges");

	SurfaceMergeEdges(a);

	// remove obsolete vertices
	Set<int> vv;
	foreach(wb, w)
		vv.add(w);
	foreachb(vv, ww){
		a->Vertex.erase(ww);
		m->Vertex[ww].Surface = -1;
		AddSubAction(new ActionModel__DeleteVertex(ww), m);
	}

	// update normals
	foreach(a->Triangle, t){
		for (int k=0;k<3;k++)
			for (int i=0;i<wa.num;i++)
				if (t.Vertex[k] == wa[i])
					t.NormalDirty = true;
	}
	a->TestSanity("AutoWeld post a");
	a->UpdateClosed();*/

	msg_db_l(1);
	//return a;
}

ActionModelSurfaceAutoWeld::~ActionModelSurfaceAutoWeld()
{
	// TODO Auto-generated destructor stub
}
