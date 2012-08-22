/*
 * ActionModelMoveSkinVertices.cpp
 *
 *  Created on: 15.03.2012
 *      Author: michi
 */

#include "ActionModelMoveSkinVertices.h"
#include "../../../../Data/Model/DataModel.h"
#include "../../../../lib/file/file.h"
#include "../../../../lib/types/types.h"
#include "../../../../Mode/Model/Mesh/ModeModelMeshTexture.h"

ActionModelMoveSkinVertices::ActionModelMoveSkinVertices(Data *d, const vector &_pos0) :
	ActionMultiView(d, _pos0)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	material = m->CurrentMaterial;
	texture_level = m->CurrentTextureLevel;

	// list of selected skin vertices and save old pos
	mode_model_mesh_texture->GetSelectedSkinVertices(surface, tria, index);
	foreachi(index, k, i){
		ModeModelTriangle &t = m->Surface[surface[i]].Triangle[tria[i]];
		old_data.add(t.SkinVertex[texture_level][k]);
	}
}

ActionModelMoveSkinVertices::~ActionModelMoveSkinVertices()
{
}



void *ActionModelMoveSkinVertices::execute(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	foreachi(index, k, ii){
		ModeModelTriangle &t = m->Surface[surface[ii]].Triangle[tria[ii]];
		vector &v = t.SkinVertex[texture_level][k];
		v = old_data[ii] + param;
	}
	return NULL;
}



void ActionModelMoveSkinVertices::undo(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	foreachi(index, k, i){
		ModeModelTriangle &t = m->Surface[surface[i]].Triangle[tria[i]];
		vector &v = t.SkinVertex[texture_level][k];
		v = old_data[i];
	}
}


