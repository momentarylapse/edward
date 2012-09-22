/*
 * ActionModelScaleSkinVertices.cpp
 *
 *  Created on: 15.03.2012
 *      Author: michi
 */

#include "ActionModelScaleSkinVertices.h"
#include "../../../../Data/Model/DataModel.h"
#include "../../../../lib/file/file.h"
#include "../../../../lib/types/types.h"
#include "../../../../Mode/Model/Mesh/ModeModelMeshTexture.h"

ActionModelScaleSkinVertices::ActionModelScaleSkinVertices(Data *d, const vector &_pos0) :
	ActionMultiView(d, _pos0)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	material = m->CurrentMaterial;
	texture_level = m->CurrentTextureLevel;

	// list of selected skin vertices and save old pos
	mode_model_mesh_texture->GetSelectedSkinVertices(surface, tria, index);
	foreachi(int k, index, i){
		ModelTriangle &t = m->Surface[surface[i]].Triangle[tria[i]];
		old_data.add(t.SkinVertex[texture_level][k]);
	}
}

ActionModelScaleSkinVertices::~ActionModelScaleSkinVertices()
{
}



void *ActionModelScaleSkinVertices::execute(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	foreachi(int k, index, i){
		ModelTriangle &t = m->Surface[surface[i]].Triangle[tria[i]];
		vector &v = t.SkinVertex[texture_level][k];
		v = pos0 + (e[0] * (old_data[i] - pos0)) * param.x * e[0] +
		           (e[1] * (old_data[i] - pos0)) * param.y * e[1] +
		           (e[2] * (old_data[i] - pos0)) * param.z * e[2];
	}
	return NULL;
}



void ActionModelScaleSkinVertices::undo(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	foreachi(int k, index, i){
		ModelTriangle &t = m->Surface[surface[i]].Triangle[tria[i]];
		vector &v = t.SkinVertex[texture_level][k];
		v = old_data[i];
	}
}


