/*
 * ActionModelSetNormalModeAll.cpp
 *
 *  Created on: 08.06.2012
 *      Author: michi
 */

#include "ActionModelSetNormalModeAll.h"
#include <assert.h>

ActionModelSetNormalModeAll::ActionModelSetNormalModeAll(int _mode)
{
	mode = _mode;
}

ActionModelSetNormalModeAll::~ActionModelSetNormalModeAll()
{
}

void ActionModelSetNormalModeAll::undo(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);

	// mode all
	if (old_mode_all != NormalModePerVertex){
		foreach(m->Vertex, v)
			v.NormalMode = old_mode_all;
	}else{
		assert(m->Vertex.num == old_mode.num);
		// per vertex
		foreachi(m->Vertex, v, i)
			v.NormalMode = old_mode[i];
	}
	m->NormalModeAll = old_mode_all;

	m->SetAllNormalsDirty();
}



void *ActionModelSetNormalModeAll::execute(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);

	// mode all
	old_mode_all = m->NormalModeAll;
	old_mode.clear();
	if (m->NormalModeAll != NormalModePerVertex){
		foreach(m->Vertex, v)
			old_mode.add(v.NormalMode);
	}

	m->NormalModeAll = mode;

	// update vertices
	foreach(m->Vertex, v)
		v.NormalMode = mode;
	m->SetAllNormalsDirty();

	return NULL;
}


