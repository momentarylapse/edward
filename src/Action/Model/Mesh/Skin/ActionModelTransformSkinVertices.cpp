/*
 * ActionModelTransformSkinVertices.cpp
 *
 *  Created on: 15.03.2012
 *      Author: michi
 */

#include "ActionModelTransformSkinVertices.h"
#include "../../../../Data/Model/DataModel.h"
#include "../../../../Data/Model/ModelMesh.h"
#include "../../../../Data/Model/ModelPolygon.h"
#include "../../../../lib/math/math.h"
#include "../../../../Mode/Model/Mesh/ModeModelMeshTexture.h"

ActionModelTransformSkinVertices::ActionModelTransformSkinVertices(DataModel *d, int _texture_level) :
	ActionMultiView()
{
	texture_level = _texture_level;

	// list of selected skin vertices and save old pos
	mode_model_mesh_texture->getSelectedSkinVertices(tria, index);
	foreachi(int k, index, i) {
		ModelPolygon &t = d->mesh->polygon[tria[i]];
		old_data.add(t.side[k].skin_vertex[texture_level]);
	}
}

void *ActionModelTransformSkinVertices::execute(Data *d) {
	DataModel *m = dynamic_cast<DataModel*>(d);
	foreachi(int k, index, ii) {
		ModelPolygon &t = m->mesh->polygon[tria[ii]];
		vector &v = t.side[k].skin_vertex[texture_level];
		v = mat * v;
	}
	return NULL;
}



void ActionModelTransformSkinVertices::undo(Data *d) {
	DataModel *m = dynamic_cast<DataModel*>(d);
	foreachi(int k, index, i) {
		ModelPolygon &t = m->mesh->polygon[tria[i]];
		vector &v = t.side[k].skin_vertex[texture_level];
		v = old_data[i];
	}
}

const string& ActionModelTransformSkinVertices::message() {
	return DataModel::MESSAGE_SKIN_CHANGE;
}
