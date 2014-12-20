/*
 * MeshSelectionMode.cpp
 *
 *  Created on: 18.12.2014
 *      Author: michi
 */

#include "MeshSelectionMode.h"
#include "../../../Edward.h"
#include "ModeModelMeshTexture.h"

MeshSelectionMode::MeshSelectionMode(ModeModelMesh *_parent)
{
	parent = _parent;
	data = parent->data;
	multi_view = parent->multi_view;
}

bool MeshSelectionMode::isActive()
{
	return mode_model_mesh->selection_mode == this;
}
