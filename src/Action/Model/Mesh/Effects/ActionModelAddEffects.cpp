/*
 * ActionModelAddEffects.cpp
 *
 *  Created on: 26.09.2012
 *      Author: michi
 */

#include "ActionModelAddEffects.h"
#include "../../../../Data/Model/ModelMesh.h"

ActionModelAddEffects::ActionModelAddEffects(DataModel *m, const ModelEffect& _effect) {
	effect = _effect;
	foreachi(ModelVertex &v, m->mesh->vertex, i)
		if (v.is_selected)
			vertex.add(i);
}

void* ActionModelAddEffects::execute(Data* d) {
	DataModel *m = dynamic_cast<DataModel*>(d);
	for (int v: vertex) {
		ModelEffect fx = effect;
		fx.vertex = v;
		m->fx.add(fx);
	}
	return NULL;
}

void ActionModelAddEffects::undo(Data* d) {
	DataModel *m = dynamic_cast<DataModel*>(d);
	m->fx.resize(m->fx.num - vertex.num);
}


