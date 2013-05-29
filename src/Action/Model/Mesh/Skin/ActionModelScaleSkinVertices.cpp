/*
 * ActionModelScaleSkinVertices.cpp
 *
 *  Created on: 15.03.2012
 *      Author: michi
 */

#include "ActionModelScaleSkinVertices.h"
#include "../../../../Data/Model/DataModel.h"
#include "../../../../lib/file/file.h"
#include "../../../../lib/math/math.h"
#include "../../../../Mode/Model/Mesh/ModeModelMeshTexture.h"

ActionModelScaleSkinVertices::ActionModelScaleSkinVertices(DataModel *d, const vector &_param, const vector &_pos0, int _texture_level) :
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

ActionModelScaleSkinVertices::~ActionModelScaleSkinVertices()
{
}



void *ActionModelScaleSkinVertices::execute(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	foreachi(int k, index, i){
		ModelPolygon &t = m->Surface[surface[i]].Polygon[tria[i]];
		vector &v = t.Side[k].SkinVertex[texture_level];
		v = pos0 + (e_x * (old_data[i] - pos0)) * param.x * e_x +
		           (e_y * (old_data[i] - pos0)) * param.y * e_y;
	}
	return NULL;
}



void ActionModelScaleSkinVertices::undo(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	foreachi(int k, index, i){
		ModelPolygon &t = m->Surface[surface[i]].Polygon[tria[i]];
		vector &v = t.Side[k].SkinVertex[texture_level];
		v = old_data[i];
	}
}


