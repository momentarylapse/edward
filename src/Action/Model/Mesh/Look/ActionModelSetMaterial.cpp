/*
 * ActionModelSetMaterial.cpp
 *
 *  Created on: 12.03.2012
 *      Author: michi
 */

#include "ActionModelSetMaterial.h"
#include "../../../../Data/Model/DataModel.h"
#include "../../../../Data/Model/ModelMesh.h"
#include "../../../../Data/Model/ModelPolygon.h"

ActionModelSetMaterial::ActionModelSetMaterial(DataModel *m, int _material)
{
	material = _material;

	// save old data
	foreachi(ModelPolygon &t, m->mesh->polygon, ti)
		if (t.is_selected){
			triangle.add(ti);
			old_material.add(t.material);
		}
}


void *ActionModelSetMaterial::execute(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);


	for (int i: triangle){
		ModelPolygon &t = m->mesh->polygon[i];
		t.material = material;
	}
	return NULL;
}



void ActionModelSetMaterial::undo(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);

	foreachi (int i, triangle, k){
		ModelPolygon &t = m->mesh->polygon[i];
		t.material = old_material[k];
	}
}


