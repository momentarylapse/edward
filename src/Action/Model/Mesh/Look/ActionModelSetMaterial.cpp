/*
 * ActionModelSetMaterial.cpp
 *
 *  Created on: 12.03.2012
 *      Author: michi
 */

#include "ActionModelSetMaterial.h"
#include "../../../../Data/Model/DataModel.h"

ActionModelSetMaterial::ActionModelSetMaterial(DataModel *m, int _material)
{
	material = _material;

	// save old data
	foreachi(ModelSurface &s, m->surface, si)
		foreachi(ModelPolygon &t, s.polygon, ti)
			if (t.is_selected){
				surface.add(si);
				triangle.add(ti);
				old_material.add(t.material);
			}
}

ActionModelSetMaterial::~ActionModelSetMaterial()
{
}



void *ActionModelSetMaterial::execute(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);


	foreachi(int s, surface, i){
		ModelPolygon &t = m->surface[s].polygon[triangle[i]];
		t.material = material;
	}
	return NULL;
}



void ActionModelSetMaterial::undo(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);

	foreachi(int s, surface, i){
		ModelPolygon &t = m->surface[s].polygon[triangle[i]];
		t.material = old_material[i];
	}
}


