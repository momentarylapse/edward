/*
 * ModeModelMesh.cpp
 *
 *  Created on: 04.03.2012
 *      Author: michi
 */

#include "../../../Edward.h"
#include "ModeModelMesh.h"
#include "ModeModelMeshVertex.h"
#include "ModeModelMeshSkin.h"

ModeModelMesh *mode_model_mesh = NULL;

ModeModelMesh::ModeModelMesh(Mode *_parent, DataModel *_data)
{
	parent = _parent;
	data = _data;
	menu = NULL;
	multi_view = NULL;

	mode_model_mesh_vertex = new ModeModelMeshVertex(this, data);
	mode_model_mesh_skin = new ModeModelMeshSkin(this, data);
}

ModeModelMesh::~ModeModelMesh()
{
}

void ModeModelMesh::Start()
{
	msg_write("model mesh start");
	ed->SetMode(mode_model_mesh_vertex);
}

void ModeModelMesh::End()
{
	msg_write("model mesh end");
}

void ModeModelMesh::OnMiddleButtonUp()
{
}



void ModeModelMesh::OnRightButtonUp()
{
}



void ModeModelMesh::OnLeftButtonUp()
{
}



void ModeModelMesh::OnLeftButtonDown()
{
}



void ModeModelMesh::OnKeyDown()
{
}



void ModeModelMesh::OnKeyUp()
{
}



void ModeModelMesh::OnCommand(const string & id)
{
}



void ModeModelMesh::OnRightButtonDown()
{
}



void ModeModelMesh::DrawWin(int win, irect dest)
{
}



void ModeModelMesh::Draw()
{
}



void ModeModelMesh::OnMouseMove()
{
}



void ModeModelMesh::OnMiddleButtonDown()
{
}



void ModeModelMesh::OnDataChange()
{
}

