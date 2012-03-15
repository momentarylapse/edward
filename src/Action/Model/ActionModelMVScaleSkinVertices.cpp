/*
 * ActionModelMVScaleSkinVertices.cpp
 *
 *  Created on: 15.03.2012
 *      Author: michi
 */

#include "ActionModelMVScaleSkinVertices.h"
#include "../../Data/Model/DataModel.h"
#include "../../lib/file/file.h"
#include "../../lib/types/types.h"
#include "../../Mode/Model/Mesh/ModeModelMeshTexture.h"

ActionModelMVScaleSkinVertices::ActionModelMVScaleSkinVertices(Data *d, const vector &_pos0) :
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

ActionModelMVScaleSkinVertices::~ActionModelMVScaleSkinVertices()
{
}

void ActionModelMVScaleSkinVertices::abort(Data *d)
{
	undo(d);
}



void *ActionModelMVScaleSkinVertices::execute(Data *d)
{
	msg_write("scale skin do");
	DataModel *m = dynamic_cast<DataModel*>(d);
	foreachi(index, i, ii){
		ModeModelSurface &s = m->Surface[surface[ii]];
		vector &v = s.Triangle[i / 3].SkinVertex[texture_level][i % 3];
		v = pos0 + (e[0] * (old_data[ii] - pos0)) * param.x * e[0] +
		           (e[1] * (old_data[ii] - pos0)) * param.y * e[1] +
		           (e[2] * (old_data[ii] - pos0)) * param.z * e[2];
	}
	return NULL;
}



void ActionModelMVScaleSkinVertices::undo(Data *d)
{
	msg_write("scale skin undo");
	DataModel *m = dynamic_cast<DataModel*>(d);
	foreachi(index, i, ii){
		ModeModelSurface &s = m->Surface[surface[ii]];
		vector &v = s.Triangle[i / 3].SkinVertex[texture_level][i % 3];
		v = old_data[ii];
	}
}



void ActionModelMVScaleSkinVertices::redo(Data *d)
{
	execute(d);
}


