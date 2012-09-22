/*
 * ActionModelSetNormalModeAll.cpp
 *
 *  Created on: 08.06.2012
 *      Author: michi
 */

#include "ActionModelSetNormalModeAll.h"
#include "../../../../Data/Model/DataModel.h"
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
		foreach(ModelVertex &v, m->Vertex)
			v.NormalMode = old_mode_all;
	}else{
		assert(m->Vertex.num == old_mode.num);
		// per vertex
		foreachi(ModelVertex &v, m->Vertex, i)
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
		foreach(ModelVertex &v, m->Vertex)
			old_mode.add(v.NormalMode);
	}

	m->NormalModeAll = mode;

	// update vertices
	foreach(ModelVertex &v, m->Vertex)
		v.NormalMode = mode;
	m->SetAllNormalsDirty();

	return NULL;
}


