/*
 * ActionModelAddEffects.cpp
 *
 *  Created on: 26.09.2012
 *      Author: michi
 */

#include "ActionModelAddEffects.h"

ActionModelAddEffects::ActionModelAddEffects(DataModel *m, const ModelEffect& _effect)
{
	effect = _effect;
	foreachi(ModelVertex &v, m->Vertex, i)
		if (v.is_selected)
			vertex.add(i);
}

void* ActionModelAddEffects::execute(Data* d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	foreach(int v, vertex){
		ModelEffect fx = effect;
		fx.Vertex = v;
		m->Fx.add(fx);
	}
	return NULL;
}

void ActionModelAddEffects::undo(Data* d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	m->Fx.resize(m->Fx.num - vertex.num);
}


