/*
 * ActionModelPasteGeometry.cpp
 *
 *  Created on: 07.06.2012
 *      Author: michi
 */

#include "ActionModelPasteGeometry.h"
#include "Vertex/ActionModelAddVertex.h"
#include "Triangle/ActionModelAddTriangle.h"
#include "../../../Data/Model/DataModel.h"

ActionModelPasteGeometry::ActionModelPasteGeometry(DataModel *m, ModelGeometry &geo)
{
	m->ClearSelection();

	int nv = m->Vertex.num;
	foreach(ModelVertex &v, m->Vertex)
		v.is_selected = false;

	foreach(ModelVertex &v, geo.Vertex)
		AddSubAction(new ActionModelAddVertex(v.pos), m);

	foreach(ModelPolygon &t, geo.Polygon){
		Array<int> v;
		for (int k=0;k<t.Side.num;k++)
			v.add(nv + t.Side[k].Vertex);
		Array<vector> sv;
		for (int l=0;l<MODEL_MAX_TEXTURES;l++)
			for (int k=0;k<t.Side.num;k++)
				sv.add(t.Side[k].SkinVertex[l]);
		AddSubAction(new ActionModelAddTriangle(m, v, t.Material, sv), m);
	}
}

ActionModelPasteGeometry::~ActionModelPasteGeometry()
{
}
