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
#include "../../ModeCreation.h"
#include "Creation/ModeModelMeshCreateVertex.h"
#include "Creation/ModeModelMeshCreateTriangles.h"
#include "Creation/ModeModelMeshCreateBall.h"
#include "Creation/ModeModelMeshCreateCube.h"
#include "Creation/ModeModelMeshCreateCylinder.h"
#include "Creation/ModeModelMeshCreatePlane.h"

ModeModelMesh *mode_model_mesh = NULL;

ModeModelMesh::ModeModelMesh(Mode *_parent, DataModel *_data)
{
	name = "ModelMesh";
	parent = _parent;
	data = _data;
	menu = NULL;
	multi_view = NULL;
	Subscribe(data);

	mode_model_mesh_vertex = new ModeModelMeshVertex(this, data);
	mode_model_mesh_skin = new ModeModelMeshSkin(this, data);
}

ModeModelMesh::~ModeModelMesh()
{
}

void ModeModelMesh::Start()
{
	ed->SetMode(mode_model_mesh_vertex);
	//ed->SetMode(mode_model_mesh_skin);
}

void ModeModelMesh::End()
{
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
	if (id == "new_point"){
		ed->SetMode(mode_model_mesh_vertex);
		ed->SetCreationMode(new ModeModelMeshCreateVertex(ed->cur_mode, data));
	}
	if (id == "new_tria"){
		ed->SetMode(mode_model_mesh_vertex);
		ed->SetCreationMode(new ModeModelMeshCreateTriangles(ed->cur_mode, data));
	}
	if (id == "new_ball")
		ed->SetCreationMode(new ModeModelMeshCreateBall(ed->cur_mode, data));
	if (id == "new_cube")
		ed->SetCreationMode(new ModeModelMeshCreateCube(ed->cur_mode, data));
	if (id == "new_cylinder")
		ed->SetCreationMode(new ModeModelMeshCreateCylinder(ed->cur_mode, data));
	if (id == "new_plane")
		ed->SetCreationMode(new ModeModelMeshCreatePlane(ed->cur_mode, data));
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



void ModeModelMesh::OnUpdate(Observable *o)
{
}

