/*
 * ActionModelMVMirrorSkinVertices.cpp
 *
 *  Created on: 15.03.2012
 *      Author: michi
 */

#include "ActionModelMVMirrorSkinVertices.h"
#include "../../../../Data/Model/DataModel.h"
#include "../../../../lib/file/file.h"
#include "../../../../lib/types/types.h"
#include "../../../../Mode/Model/Mesh/ModeModelMeshTexture.h"

ActionModelMVMirrorSkinVertices::ActionModelMVMirrorSkinVertices(Data *d, const vector &_pos0) :
	ActionMultiView(d, _pos0)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	material = m->CurrentMaterial;
	texture_level = m->CurrentTextureLevel;

	// list of selected skin vertices and save old pos
	mode_model_mesh_texture->GetSelectedSkinVertices(surface, index);
	foreachi(index, i, ii){
		ModeModelSurface &s = m->Surface[surface[ii]];
		old_data.add(s.Triangle[i / 3].SkinVertex[texture_level][i % 3]);
	}
}

ActionModelMVMirrorSkinVertices::~ActionModelMVMirrorSkinVertices()
{
}



void ActionModelMVMirrorSkinVertices::abort(Data *d)
{
	undo(d);
}



void *ActionModelMVMirrorSkinVertices::execute(Data *d)
{
	msg_write("mirror skin do");
	DataModel *m = dynamic_cast<DataModel*>(d);
	foreachi(index, i, ii){
		ModeModelSurface &s = m->Surface[surface[ii]];
		vector &v = s.Triangle[i / 3].SkinVertex[texture_level][i % 3];
		v = old_data[ii] - 2 * param * ((old_data[ii] - pos0) * param);
	}
	return NULL;
}



void ActionModelMVMirrorSkinVertices::undo(Data *d)
{
	msg_write("mirror skin undo");
	DataModel *m = dynamic_cast<DataModel*>(d);
	foreachi(index, i, ii){
		ModeModelSurface &s = m->Surface[surface[ii]];
		vector &v = s.Triangle[i / 3].SkinVertex[texture_level][i % 3];
		v = old_data[ii];
	}
}


