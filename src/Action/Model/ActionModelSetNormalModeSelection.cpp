/*
 * ActionModelSetNormalModeSelection.cpp
 *
 *  Created on: 12.03.2012
 *      Author: michi
 */

#include "ActionModelSetNormalModeSelection.h"

ActionModelSetNormalModeSelection::ActionModelSetNormalModeSelection(DataModel *m, int _mode)
{
	mode = _mode;
	foreachi(m->Vertex, v, i)
		if (v.is_selected)
			index.add(i);
}

ActionModelSetNormalModeSelection::~ActionModelSetNormalModeSelection()
{
}

void *ActionModelSetNormalModeSelection::execute(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);

	// mode all
	old_mode_all = m->NormalModeAll;
	if (m->NormalModeAll != NormalModePerVertex){
		foreach(m->Vertex, v)
			v.NormalMode = mode;
		m->NormalModeAll = NormalModePerVertex;
	}

	// per vertex
	old_mode.clear();
	foreach(index, i){
		old_mode.add(m->Vertex[i].NormalMode);
		m->Vertex[i].NormalMode = mode;
	}
	m->SetNormalsDirtyByVertices(index);

	return NULL;
}



void ActionModelSetNormalModeSelection::undo(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);


	// mode all
	if (old_mode_all != NormalModePerVertex){
		m->NormalModeAll = old_mode_all;
		foreach(m->Vertex, v)
			v.NormalMode = old_mode_all;
	}else{
		// per vertex
		foreach(index, i)
			m->Vertex[i].NormalMode = old_mode[i];
	}

	m->SetNormalsDirtyByVertices(index);
}


