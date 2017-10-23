/*
 * ActionModelEditMaterial.cpp
 *
 *  Created on: 06.06.2012
 *      Author: michi
 */

#include <algorithm>
#include "ActionModelEditMaterial.h"
#include <assert.h>

ActionModelEditMaterial::ActionModelEditMaterial(int _index, const ModelMaterial &_mat)
{
	index = _index;
	mat = _mat;
}

void *ActionModelEditMaterial::execute(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);

	assert((index >= 0) && (index < m->material.num));

	std::swap(mat, m->material[index]);

	// correct skin vertices
	// (copy highest texture level when adding more levels)
	for (ModelSurface &s: m->surface)
		for (ModelPolygon &p: s.polygon){
			if (p.material == index){
				for (ModelPolygonSide &side: p.side)
					for (int l=mat.texture_files.num;l<m->material[index].texture_files.num;l++)
						side.skin_vertex[l] = side.skin_vertex[mat.texture_files.num - 1];
			}
		}

	return NULL;
}



void ActionModelEditMaterial::undo(Data *d)
{
	execute(d);
}


