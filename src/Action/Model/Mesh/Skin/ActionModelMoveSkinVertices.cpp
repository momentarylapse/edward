/*
 * ActionModelMoveSkinVertices.cpp
 *
 *  Created on: 15.03.2012
 *      Author: michi
 */

#include "ActionModelMoveSkinVertices.h"
#include "../../../../Data/Model/DataModel.h"
#include "../../../../lib/types/types.h"
#include "../../../../Mode/Model/Mesh/ModeModelMeshTexture.h"

ActionModelMoveSkinVertices::ActionModelMoveSkinVertices(DataModel *d, const vector &_param, const vector &_pos0) :
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

ActionModelMoveSkinVertices::~ActionModelMoveSkinVertices()
{
}



void *ActionModelMoveSkinVertices::execute(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	foreachi(int k, index, ii){
		ModelPolygon &t = m->Surface[surface[ii]].Polygon[tria[ii]];
		vector &v = t.Side[k].SkinVertex[texture_level];
		v += param;
	}
	return NULL;
}



void ActionModelMoveSkinVertices::undo(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	foreachi(int k, index, i){
		ModelPolygon &t = m->Surface[surface[i]].Polygon[tria[i]];
		vector &v = t.Side[k].SkinVertex[texture_level];
		v = old_data[i];
	}
}


