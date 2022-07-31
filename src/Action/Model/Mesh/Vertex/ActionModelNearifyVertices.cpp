/*
 * ActionModelNearifyVertices.cpp
 *
 *  Created on: 10.06.2012
 *      Author: michi
 */

#include "ActionModelNearifyVertices.h"
#include "../../../../Data/Model/DataModel.h"
#include "../../../../Data/Model/ModelMesh.h"
#include <assert.h>

ActionModelNearifyVertices::ActionModelNearifyVertices(DataModel *m)
{
	foreachi(ModelVertex &v, m->edit_mesh->vertex, i)
		if (v.is_selected)
			index.add(i);
}

ActionModelNearifyVertices::~ActionModelNearifyVertices()
{
}

void ActionModelNearifyVertices::undo(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	assert(index.num == old_pos.num);

	// apply old data
	foreachi(int vi, index, i)
		m->edit_mesh->vertex[vi].pos = old_pos[i];

	m->set_normals_dirty_by_vertices(index);
}



void *ActionModelNearifyVertices::execute(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);

	old_pos.clear();
	vec3 v = v_0;

	for (int i: index){
		// save old data
		old_pos.add(m->edit_mesh->vertex[i].pos);

		// average
		v += m->edit_mesh->vertex[i].pos;
	}

	if (index.num > 0)
		v /= index.num;

	// apply
	for (int i: index)
		m->edit_mesh->vertex[i].pos = v;

	m->set_normals_dirty_by_vertices(index);
	return NULL;
}

bool ActionModelNearifyVertices::was_trivial()
{	return index.num == 0;	}


