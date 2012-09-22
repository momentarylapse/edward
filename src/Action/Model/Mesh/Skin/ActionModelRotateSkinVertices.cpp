/*
 * ActionModelRotateSkinVertices.cpp
 *
 *  Created on: 15.03.2012
 *      Author: michi
 */

#include "ActionModelRotateSkinVertices.h"
#include "../../../../Data/Model/DataModel.h"
#include "../../../../lib/file/file.h"
#include "../../../../lib/types/types.h"
#include "../../../../Mode/Model/Mesh/ModeModelMeshTexture.h"

ActionModelRotateSkinVertices::ActionModelRotateSkinVertices(Data *d, const vector &_pos0) :
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

ActionModelRotateSkinVertices::~ActionModelRotateSkinVertices()
{
}



void *ActionModelRotateSkinVertices::execute(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	matrix rot;
	MatrixRotation(rot, param);
	foreachi(int k, index, i){
		ModelTriangle &t = m->Surface[surface[i]].Triangle[tria[i]];
		vector &v = t.SkinVertex[texture_level][k];
		v = pos0 + rot * (old_data[i] - pos0);
	}
	return NULL;
}



void ActionModelRotateSkinVertices::undo(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	foreachi(int k, index, i){
		ModelTriangle &t = m->Surface[surface[i]].Triangle[tria[i]];
		vector &v = t.SkinVertex[texture_level][k];
		v = old_data[i];
	}
}


