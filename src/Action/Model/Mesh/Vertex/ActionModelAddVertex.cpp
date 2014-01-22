/*
 * ActionAddVertex.cpp
 *
 *  Created on: 04.03.2012
 *      Author: michi
 */

#include "ActionModelAddVertex.h"
#include "../../../../Data/Model/DataModel.h"
#include "../../../../Edward.h"
#include "../../../../MultiView/MultiView.h"
#include <assert.h>

ActionModelAddVertex::ActionModelAddVertex(const vector &_pos, int _bone_index, int _normal_mode)
{
	pos = _pos;
	bone_index = _bone_index;
	normal_mode = _normal_mode;
}

ActionModelAddVertex::~ActionModelAddVertex()
{
}



void *ActionModelAddVertex::execute(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);

	// new vertex
	ModelVertex vv;
	vv.pos = pos;
	if (normal_mode < 0)
		vv.NormalMode = NormalModeAngular;
	else
		vv.NormalMode = normal_mode;
	vv.BoneIndex = bone_index;
	vv.is_selected = false;
	vv.is_special = false;
	vv.view_stage = ed->multi_view_3d->view_stage;
	vv.RefCount = 0;
	vv.Surface = -1;
	m->Vertex.add(vv);

	// correct animations
	foreach(ModelMove &move, m->Move)
		if (move.Type == MoveTypeVertex)
			foreach(ModelFrame &f, move.Frame)
				f.VertexDPos.resize(m->Vertex.num);
	return &m->Vertex.back();
}



void ActionModelAddVertex::undo(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	assert(m->Vertex.back().RefCount == 0);
	assert(m->Vertex.back().Surface < 0);

	// delete
	m->Vertex.pop();

	// correct animations
	foreach(ModelMove &move, m->Move)
		if (move.Type == MoveTypeVertex){
			foreach(ModelFrame &f, move.Frame)
				f.VertexDPos.resize(m->Vertex.num);
		}
}

