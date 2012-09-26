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
	foreachi(ModelSurface &s, m->Surface, si)
		foreachi(ModelPolygon &t, s.Polygon, ti)
			if (t.is_selected){
				surface.add(si);
				triangle.add(ti);
				old_material.add(t.Material);
			}
}

ActionModelSetMaterial::~ActionModelSetMaterial()
{
}



void *ActionModelSetMaterial::execute(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);


	foreachi(int s, surface, i){
		ModelPolygon &t = m->Surface[s].Polygon[triangle[i]];
		t.Material = material;
	}
	return NULL;
}



void ActionModelSetMaterial::undo(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);

	foreachi(int s, surface, i){
		ModelPolygon &t = m->Surface[s].Polygon[triangle[i]];
		t.Material = old_material[i];
	}
}


