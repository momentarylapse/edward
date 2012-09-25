/*
 * ActionModelClearEffects.cpp
 *
 *  Created on: 26.09.2012
 *      Author: michi
 */

#include "ActionModelClearEffects.h"

ActionModelClearEffects::ActionModelClearEffects(DataModel *m)
{
	foreachi(ModelEffect &fx, m->Fx, i)
		if (m->Vertex[fx.Vertex].is_selected){
			effects.add(fx);
			index.add(i);
		}
}

void* ActionModelClearEffects::execute(Data* d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	foreachb(int i, index)
		m->Fx.erase(i);
	return NULL;
}

void ActionModelClearEffects::undo(Data* d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	foreachi(int i, index, ii)
		m->Fx.insert(effects[ii], i);
}


