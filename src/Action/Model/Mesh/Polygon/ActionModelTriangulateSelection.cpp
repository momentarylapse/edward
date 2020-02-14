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
#include "../../../../Data/Model/ModelMesh.h"
#include "../../../../Data/Model/ModelPolygon.h"

ActionModelTriangulateSelection::ActionModelTriangulateSelection()
{
}

void *ActionModelTriangulateSelection::compose(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
		foreachib(ModelPolygon &t, m->edit_mesh->polygon, ti)
			if (t.is_selected){
				if (t.side.num == 3)
					continue;

				// save old polygon
				ModelPolygon temp = t;

				// delete old polygon
				addSubAction(new ActionModelSurfaceDeletePolygon(ti), m);

				// triangulate
				Array<int> vv = temp.triangulate(m->edit_mesh->vertex);
				for (int i=0;i<vv.num/3;i++){
					Array<int> v;
					for (int k=0;k<3;k++)
						v.add(temp.side[vv[i*3+k]].vertex);
					Array<vector> sv;
					for (int l=0;l<MATERIAL_MAX_TEXTURES;l++)
						for (int k=0;k<3;k++)
							sv.add(temp.side[vv[i*3+k]].skin_vertex[l]);
					addSubAction(new ActionModelSurfaceAddPolygon(v, temp.material, sv), m);
				}


				_foreach_it_.update(); // TODO
			}
	return NULL;
}

