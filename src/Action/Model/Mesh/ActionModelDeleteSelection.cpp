/*
 * ActionModelDeleteSelection.cpp
 *
 *  Created on: 12.03.2012
 *      Author: michi
 */

#include "ActionModelDeleteSelection.h"
#include "Surface/Helper/ActionModelSurfaceDeletePolygon.h"
#include "Vertex/Helper/ActionModelDeleteUnusedVertex.h"
#include "Surface/Helper/ActionModelDeleteEmptySurface.h"
#include "../../../Data/Model/DataModel.h"

ActionModelDeleteSelection::ActionModelDeleteSelection(bool _greedy)
{
	greedy = _greedy;
}

void *ActionModelDeleteSelection::compose(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);

	foreachib(ModelSurface &s, m->surface, si){
		foreachib(ModelPolygon &t, s.polygon, ti){
			bool del = false;
			if (greedy){
				for (int k=0;k<t.side.num;k++)
					del |= m->vertex[t.side[k].vertex].is_selected;
			}else{
				del = t.is_selected;
			}
			if (del)
				addSubAction(new ActionModelSurfaceDeletePolygon(si, ti), m);
			_foreach_it_.update(); // TODO
		}

		if (s.polygon.num == 0)
			addSubAction(new ActionModelDeleteEmptySurface(si), m);
		_foreach_it_.update(); // TODO
	}

	foreachib(ModelVertex &v, m->vertex, i)
		if (v.is_selected)
			if (v.ref_count == 0){
				addSubAction(new ActionModelDeleteUnusedVertex(i), m);
				_foreach_it_.update(); // TODO
			}

	return NULL;
}
