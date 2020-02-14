/*
 * ActionModelAddPolygonAutoSkin.cpp
 *
 *  Created on: 26.09.2012
 *      Author: michi
 */

#include "ActionModelAddPolygonAutoSkin.h"
#include "ActionModelAddPolygonWithSkinGenerator.h"
#include "../../../../Data/Model/DataModel.h"
#include "../../../../Data/Model/ModelMesh.h"
#include "../../../../Data/Model/SkinGenerator.h"

ActionModelAddPolygonAutoSkin::ActionModelAddPolygonAutoSkin(Array<int> &_vertex, int _material) {
	vertex = _vertex;
	material = _material;
}

void *ActionModelAddPolygonAutoSkin::compose(Data *d) {
	DataModel *m = dynamic_cast<DataModel*>(d);

	SkinGenerator sg;
	sg.init_point_cloud_boundary(m->edit_mesh->vertex, vertex);

	return addSubAction(new ActionModelAddPolygonWithSkinGenerator(vertex, material, sg), m);
}

