/*
 * ActionAddVertex.cpp
 *
 *  Created on: 04.03.2012
 *      Author: michi
 */

#include "ActionAddVertex.h"
#include "../../Data/Model/DataModel.h"

ActionAddVertex::ActionAddVertex(const vector & _v, int _skin)
{
	v = _v;
	skin = _skin;
}

ActionAddVertex::~ActionAddVertex()
{
}



void ActionAddVertex::execute(Data *d)
{
	redo(d);
}



void ActionAddVertex::undo(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	m->Skin[skin].Vertex.pop();
}



void ActionAddVertex::redo(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	ModeModelVertex vv;
	vv.pos = v;
	vv.BoneIndex = -1;
	vv.RefCount = -1;
	vv.NormalDirty = true;
	vv.Surface = -1;
	vv.is_selected = vv.is_special = false;
	vv.view_stage = 0;
	m->Skin[skin].Vertex.add(vv);
}

