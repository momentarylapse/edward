/*
 * ActionModelDeleteSelection.cpp
 *
 *  Created on: 12.03.2012
 *      Author: michi
 */

#include "ActionModelDeleteSelection.h"
#include "Surface/Helper/ActionModel__SurfaceDeleteTriangle.h"
#include "Vertex/Helper/ActionModelDeleteUnusedVertex.h"
#include "Surface/Helper/ActionModel__SurfaceDeleteTriangle.h"
#include "Surface/Helper/ActionModelDeleteEmptySurface.h"

ActionModelDeleteSelection::ActionModelDeleteSelection(DataModel *m, bool greedy)
{
	foreachbi(m->Surface, s, si){
		foreachbi(s.Triangle, t, ti){
			bool del = false;
			if (greedy){
				for (int k=0;k<3;k++)
					del |= m->Vertex[t.Vertex[k]].is_selected;
			}else{
				del = t.is_selected;
			}
			if (del)
				AddSubAction(new ActionModel__SurfaceDeleteTriangle(si, ti), m);
		}

		if (s.Triangle.num == 0)
			AddSubAction(new ActionModelDeleteEmptySurface(si), m);
	}

	foreachbi(m->Vertex, v, i)
		if (v.is_selected)
			if (v.RefCount == 0)
				AddSubAction(new ActionModelDeleteUnusedVertex(i), m);
}

ActionModelDeleteSelection::~ActionModelDeleteSelection()
{
}
