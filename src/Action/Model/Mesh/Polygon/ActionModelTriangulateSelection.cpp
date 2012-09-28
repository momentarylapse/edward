/*
 * ActionModelTriangulateSelection.cpp
 *
 *  Created on: 26.09.2012
 *      Author: michi
 */

#include "ActionModelTriangulateSelection.h"
#include "../Surface/Helper/ActionModelSurfaceDeletePolygon.h"
#include "../Surface/Helper/ActionModelSurfaceAddPolygon.h"
#include "../../../../Data/Model/DataModel.h"

ActionModelTriangulateSelection::ActionModelTriangulateSelection()
{
}

void *ActionModelTriangulateSelection::compose(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	foreachi(ModelSurface &s, m->Surface, si){
		foreachib(ModelPolygon &t, s.Polygon, ti)
			if (t.is_selected){
				if (t.Side.num == 3)
					continue;

				// save old polygon
				ModelPolygon temp = t;

				// delete old polygon
				AddSubAction(new ActionModelSurfaceDeletePolygon(si, ti), m);

				// triangulate
				Array<int> vv = temp.Triangulate(m);
				for (int i=0;i<vv.num/3;i++){
					Array<int> v;
					for (int k=0;k<3;k++)
						v.add(temp.Side[vv[i*3+k]].Vertex);
					Array<vector> sv;
					for (int l=0;l<MODEL_MAX_TEXTURES;l++)
						for (int k=0;k<3;k++)
							sv.add(temp.Side[vv[i*3+k]].SkinVertex[l]);
					AddSubAction(new ActionModelSurfaceAddPolygon(si, v, temp.Material, sv), m);
				}


				_foreach_it_.update(); // TODO
			}
	}
	return NULL;
}

