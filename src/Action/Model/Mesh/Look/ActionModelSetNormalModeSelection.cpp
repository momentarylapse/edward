/*
 * ActionModelSetNormalModeSelection.cpp
 *
 *  Created on: 12.03.2012
 *      Author: michi
 */

#include "ActionModelSetNormalModeSelection.h"
#include "../../../../Data/Model/DataModel.h"
#include "../../../../Data/Model/ModelMesh.h"
#include "../../../../Data/Model/ModelPolygon.h"

ActionModelSetNormalModeSelection::ActionModelSetNormalModeSelection(DataModel *m, int _mode)
{
	mode = _mode;
	foreachi(ModelVertex &v, m->mesh->vertex, i)
		if (v.is_selected)
			index.add(i);
}

ActionModelSetNormalModeSelection::~ActionModelSetNormalModeSelection()
{
}

void *ActionModelSetNormalModeSelection::execute(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);

	// per vertex
	old_mode.clear();
	for (int i: index){
		old_mode.add(m->mesh->vertex[i].normal_mode);
		m->mesh->vertex[i].normal_mode = mode;
	}
	m->setNormalsDirtyByVertices(index);

	return NULL;
}



void ActionModelSetNormalModeSelection::undo(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);

	// per vertex
	foreachi(int i, index, ii)
		m->mesh->vertex[i].normal_mode = old_mode[ii];

	m->setNormalsDirtyByVertices(index);
}


