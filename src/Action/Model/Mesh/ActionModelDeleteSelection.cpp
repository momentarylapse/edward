/*
 * ActionModelDeleteSelection.cpp
 *
 *  Created on: 12.03.2012
 *      Author: michi
 */

#include "ActionModelDeleteSelection.h"
#include "Surface/Helper/ActionModelSurfaceDeleteTriangle.h"
#include "Vertex/Helper/ActionModelDeleteUnusedVertex.h"
#include "Surface/Helper/ActionModelDeleteEmptySurface.h"
#include "../../../Data/Model/DataModel.h"

ActionModelDeleteSelection::ActionModelDeleteSelection(DataModel *m, bool greedy)
{
	foreachib(ModelSurface &s, m->Surface, si){
		foreachib(ModelTriangle &t, s.Triangle, ti){
			bool del = false;
			if (greedy){
				for (int k=0;k<3;k++)
					del |= m->Vertex[t.Vertex[k]].is_selected;
			}else{
				del = t.is_selected;
			}
			if (del)
				AddSubAction(new ActionModelSurfaceDeleteTriangle(si, ti), m);
			_foreach_it_.update(); // TODO
		}

		if (s.Triangle.num == 0)
			AddSubAction(new ActionModelDeleteEmptySurface(si), m);
		_foreach_it_.update(); // TODO
	}

	foreachib(ModelVertex &v, m->Vertex, i)
		if (v.is_selected)
			if (v.RefCount == 0){
				AddSubAction(new ActionModelDeleteUnusedVertex(i), m);
				_foreach_it_.update(); // TODO
			}
}

ActionModelDeleteSelection::~ActionModelDeleteSelection()
{
}
