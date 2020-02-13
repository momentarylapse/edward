/*
 * ActionModelAlignToGrid.cpp
 *
 *  Created on: 10.06.2012
 *      Author: michi
 */

#include "ActionModelAlignToGrid.h"
#include "../../../../Data/Model/DataModel.h"
#include "../../../../Data/Model/ModelMesh.h"
#include <assert.h>

ActionModelAlignToGrid::ActionModelAlignToGrid(DataModel *m, float _grid_dist)
{
	foreachi(ModelVertex &v, m->mesh->vertex, i)
		if (v.is_selected)
			index.add(i);
	grid_dist = _grid_dist;
}

ActionModelAlignToGrid::~ActionModelAlignToGrid()
{
}

void align_float(float &f, float D)
{
	if (f > 0)
		f = int(f / D + 0.5f) * D;
	else
		f = int(f / D - 0.5f) * D;
}

void *ActionModelAlignToGrid::execute(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);

	old_pos.clear();

	for (int i: index){
		// save old data
		old_pos.add(m->mesh->vertex[i].pos);

		// apply
		align_float(m->mesh->vertex[i].pos.x, grid_dist);
		align_float(m->mesh->vertex[i].pos.y, grid_dist);
		align_float(m->mesh->vertex[i].pos.z, grid_dist);
	}

	m->setNormalsDirtyByVertices(index);
	return NULL;
}



void ActionModelAlignToGrid::undo(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	assert(index.num == old_pos.num);

	// apply old data
	foreachi(int vi, index, i)
		m->mesh->vertex[vi].pos = old_pos[i];

	m->setNormalsDirtyByVertices(index);
}

bool ActionModelAlignToGrid::was_trivial()
{	return index.num == 0;	}
