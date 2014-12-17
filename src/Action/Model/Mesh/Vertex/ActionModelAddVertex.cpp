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
		vv.normal_mode = NORMAL_MODE_ANGULAR;
	else
		vv.normal_mode = normal_mode;
	vv.bone_index = bone_index;
	vv.is_selected = false;
	vv.is_special = false;
	vv.view_stage = ed->multi_view_3d->view_stage;
	vv.ref_count = 0;
	vv.surface = -1;
	m->vertex.add(vv);

	// correct animations
	foreach(ModelMove &move, m->move){
		if (move.type == MOVE_TYPE_VERTEX){
			foreach(ModelFrame &f, move.frame)
				f.vertex_dpos.resize(m->vertex.num);
		}
	}
	return &m->vertex.back();
}



void ActionModelAddVertex::undo(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	assert(m->vertex.back().ref_count == 0);
	assert(m->vertex.back().surface < 0);

	// delete
	m->vertex.pop();

	// correct animations
	foreach(ModelMove &move, m->move)
		if (move.type == MOVE_TYPE_VERTEX){
			foreach(ModelFrame &f, move.frame)
				f.vertex_dpos.resize(m->vertex.num);
		}
}

