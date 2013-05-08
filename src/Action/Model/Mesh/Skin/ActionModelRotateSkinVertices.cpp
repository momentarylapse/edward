/*
 * ActionModelRotateSkinVertices.cpp
 *
 *  Created on: 15.03.2012
 *      Author: michi
 */

#include "ActionModelRotateSkinVertices.h"
#include "../../../../Data/Model/DataModel.h"
#include "../../../../lib/math/math.h"
#include "../../../../Mode/Model/Mesh/ModeModelMeshTexture.h"

ActionModelRotateSkinVertices::ActionModelRotateSkinVertices(DataModel *d, const vector &_param, const vector &_pos0) :
	ActionMultiView(_param, _pos0)
{
	material = d->CurrentMaterial;
	texture_level = d->CurrentTextureLevel;

	// list of selected skin vertices and save old pos
	mode_model_mesh_texture->GetSelectedSkinVertices(surface, tria, index);
	foreachi(int k, index, i){
		ModelPolygon &t = d->Surface[surface[i]].Polygon[tria[i]];
		old_data.add(t.Side[k].SkinVertex[texture_level]);
	}
}

ActionModelRotateSkinVertices::~ActionModelRotateSkinVertices()
{
}



void *ActionModelRotateSkinVertices::execute(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	matrix rot;
	MatrixRotation(rot, param);
	foreachi(int k, index, i){
		ModelPolygon &t = m->Surface[surface[i]].Polygon[tria[i]];
		vector &v = t.Side[k].SkinVertex[texture_level];
		v = pos0 + rot * (old_data[i] - pos0);
	}
	return NULL;
}



void ActionModelRotateSkinVertices::undo(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	foreachi(int k, index, i){
		ModelPolygon &t = m->Surface[surface[i]].Polygon[tria[i]];
		vector &v = t.Side[k].SkinVertex[texture_level];
		v = old_data[i];
	}
}


