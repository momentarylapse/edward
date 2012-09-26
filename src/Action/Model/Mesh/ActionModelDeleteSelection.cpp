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
		foreachib(ModelPolygon &t, s.Polygon, ti){
			bool del = false;
			if (greedy){
				for (int k=0;k<t.Side.num;k++)
					del |= m->Vertex[t.Side[k].Vertex].is_selected;
			}else{
				del = t.is_selected;
			}
			if (del)
				AddSubAction(new ActionModelSurfaceDeleteTriangle(si, ti), m);
			_foreach_it_.update(); // TODO
		}

		if (s.Polygon.num == 0)
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
