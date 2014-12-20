/*
 * ActionModelPasteGeometry.cpp
 *
 *  Created on: 07.06.2012
 *      Author: michi
 */

#include "ActionModelPasteGeometry.h"
#include "Vertex/ActionModelAddVertex.h"
#include "Polygon/ActionModelAddPolygon.h"
#include "../../../Data/Model/DataModel.h"
#include "../../../Data/Model/Geometry/Geometry.h"

ActionModelPasteGeometry::ActionModelPasteGeometry(Geometry &_geo, int _default_material) :
	geo(_geo)
{
	default_material = _default_material;
}

void *ActionModelPasteGeometry::compose(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	m->clearSelection();

	int nv = m->vertex.num;
	foreach(ModelVertex &v, m->vertex)
		v.is_selected = false;

	foreach(ModelVertex &v, geo.vertex)
		addSubAction(new ActionModelAddVertex(v.pos), m);

	foreach(ModelPolygon &t, geo.polygon){
		Array<int> v;
		for (int k=0;k<t.side.num;k++)
			v.add(nv + t.side[k].vertex);
		Array<vector> sv;
		for (int l=0;l<MATERIAL_MAX_TEXTURES;l++)
			for (int k=0;k<t.side.num;k++)
				sv.add(t.side[k].skin_vertex[l]);
		int mat = (t.material >= 0) ? t.material : default_material;
		addSubAction(new ActionModelAddPolygon(v, mat, sv), m);
	}
	return NULL;
}
