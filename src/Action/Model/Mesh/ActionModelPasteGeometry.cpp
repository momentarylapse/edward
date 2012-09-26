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
#if 0
	m->ClearSelection();

	int nv = m->Vertex.num;
	foreach(ModelVertex &v, m->Vertex)
		v.is_selected = false;

	foreach(ModelVertex &v, geo.Vertex)
		AddSubAction(new ActionModelAddVertex(v.pos), m);

	foreach(ModelPolygon &t, geo.Polygon){
		vector sv[3][MODEL_MAX_TEXTURES];
		for (int k=0;k<3;k++)
			for (int l=0;l<MODEL_MAX_TEXTURES;l++)
				sv[k][l] = t.SkinVertex[l][k];
		AddSubAction(new ActionModelAddTriangle(m, nv + t.Vertex[0], nv + t.Vertex[1], nv + t.Vertex[2], t.Material, sv[0], sv[1], sv[2]), m);
	}
#endif
}

ActionModelPasteGeometry::~ActionModelPasteGeometry()
{
}
