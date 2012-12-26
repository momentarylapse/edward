/*
 * ActionModelEditMaterial.cpp
 *
 *  Created on: 06.06.2012
 *      Author: michi
 */

#include "ActionModelEditMaterial.h"
#include <assert.h>

ActionModelEditMaterial::ActionModelEditMaterial(int _index, const ModelMaterial &_mat)
{
	index = _index;
	mat = _mat;
}

ActionModelEditMaterial::~ActionModelEditMaterial()
{
}

void *ActionModelEditMaterial::execute(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);

	assert((index >= 0) && (index < m->Material.num));

	ModelMaterial old_mat = m->Material[index];
	m->Material[index] = mat;
	mat = old_mat;

	// correct skin vertices
	// (copy highest texture level when adding more levels)
	foreach(ModelSurface &s, m->Surface)
		foreach(ModelPolygon &p, s.Polygon){
			if (p.Material == index){
				foreach(ModelPolygonSide &side, p.Side)
					for (int l=mat.NumTextures;l<m->Material[index].NumTextures;l++)
						side.SkinVertex[l] = side.SkinVertex[mat.NumTextures - 1];
			}
		}

	return NULL;
}



void ActionModelEditMaterial::undo(Data *d)
{
	execute(d);
}


