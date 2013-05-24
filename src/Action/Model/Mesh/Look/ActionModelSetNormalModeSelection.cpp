/*
 * ActionModelSetNormalModeSelection.cpp
 *
 *  Created on: 12.03.2012
 *      Author: michi
 */

#include "ActionModelSetNormalModeSelection.h"
#include "../../../../Data/Model/DataModel.h"

ActionModelSetNormalModeSelection::ActionModelSetNormalModeSelection(DataModel *m, int _mode)
{
	mode = _mode;
	foreachi(ModelVertex &v, m->Vertex, i)
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
	foreach(int i, index){
		old_mode.add(m->Vertex[i].NormalMode);
		m->Vertex[i].NormalMode = mode;
	}
	m->SetNormalsDirtyByVertices(index);

	return NULL;
}



void ActionModelSetNormalModeSelection::undo(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);

	// per vertex
	foreachi(int i, index, ii)
		m->Vertex[i].NormalMode = old_mode[ii];

	m->SetNormalsDirtyByVertices(index);
}


