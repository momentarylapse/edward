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

ActionModelDeleteSelection::ActionModelDeleteSelection(const ModelSelectionState &s, bool _greedy) : sel(s)
{
	greedy = _greedy;
}

void *ActionModelDeleteSelection::compose(Data *d) {
	DataModel *m = dynamic_cast<DataModel*>(d);

		foreachib(ModelPolygon &t, m->polygon, ti) {
			bool del = false;
			if (greedy) {
				for (int k=0;k<t.side.num;k++)
					del |= sel.vertex.contains(t.side[k].vertex);
			} else {
				del = t.is_selected;
			}
			if (del)
				addSubAction(new ActionModelSurfaceDeletePolygon(ti), m);
			_foreach_it_.update(); // TODO
		}

		foreachb (int i, sel.vertex)
			if (m->vertex[i].ref_count == 0) {
				addSubAction(new ActionModelDeleteUnusedVertex(i), m);
			}

	return NULL;
}
