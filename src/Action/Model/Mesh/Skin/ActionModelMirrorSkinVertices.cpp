/*
 * ActionModelMirrorSkinVertices.cpp
 *
 *  Created on: 15.03.2012
 *      Author: michi
 */

#include "ActionModelMirrorSkinVertices.h"
#include "../../../../Data/Model/DataModel.h"
#include "../../../../lib/math/math.h"
#include "../../../../Mode/Model/Mesh/ModeModelMeshTexture.h"

ActionModelMirrorSkinVertices::ActionModelMirrorSkinVertices(DataModel *d, const vector &_param, const vector &_pos0, int _texture_level) :
	ActionMultiView(_param, _pos0)
{
	texture_level = _texture_level;

	// list of selected skin vertices and save old pos
	mode_model_mesh_texture->GetSelectedSkinVertices(surface, tria, index);
	foreachi(int k, index, i){
		ModelPolygon &t = d->Surface[surface[i]].Polygon[tria[i]];
		old_data.add(t.Side[k].SkinVertex[texture_level]);
	}
}

ActionModelMirrorSkinVertices::~ActionModelMirrorSkinVertices()
{
}



void *ActionModelMirrorSkinVertices::execute(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	foreachi(int k, index, i){
		ModelPolygon &t = m->Surface[surface[i]].Polygon[tria[i]];
		vector &v = t.Side[k].SkinVertex[texture_level];
		v = v - 2 * param * ((v - pos0) * param);
	}
	return NULL;
}



void ActionModelMirrorSkinVertices::undo(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	foreachi(int k, index, i){
		ModelPolygon &t = m->Surface[surface[i]].Polygon[tria[i]];
		vector &v = t.Side[k].SkinVertex[texture_level];
		v = old_data[i];
	}
}


