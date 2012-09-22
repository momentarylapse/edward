/*
 * ModeModelMeshBevelVertices.cpp
 *
 *  Created on: 23.09.2012
 *      Author: michi
 */

#include "ModeModelMeshBevelVertices.h"

ModeModelMeshBevelVertices::ModeModelMeshBevelVertices(Mode *_parent) :
	ModeCreation("ModelMeshBevelVertices", _parent)
{
	data = (DataModel*)_parent->GetData();

	message = _("Radius skalieren");

	radius = 0;
}

ModeModelMeshBevelVertices::~ModeModelMeshBevelVertices()
{
}

void ModeModelMeshBevelVertices::OnMouseMove()
{
}

void ModeModelMeshBevelVertices::OnLeftButtonDown()
{
}

void ModeModelMeshBevelVertices::OnDrawWin(int win, irect dest)
{
}


