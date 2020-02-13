/*
 * ActionModelClearEffects.cpp
 *
 *  Created on: 26.09.2012
 *      Author: michi
 */

#include "ActionModelClearEffects.h"
#include "../../../../Data/Model/ModelMesh.h"

ActionModelClearEffects::ActionModelClearEffects(DataModel *m)
{
	foreachi(ModelEffect &fx, m->fx, i)
		if (m->mesh->vertex[fx.vertex].is_selected){
			effects.add(fx);
			index.add(i);
		}
}

void* ActionModelClearEffects::execute(Data* d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	foreachb(int i, index)
		m->fx.erase(i);
	return NULL;
}

void ActionModelClearEffects::undo(Data* d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	foreachi(int i, index, ii)
		m->fx.insert(effects[ii], i);
}


