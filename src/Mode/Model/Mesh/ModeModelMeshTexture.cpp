/*
 * ModeModelMeshTexture.cpp
 *
 *  Created on: 14.03.2012
 *      Author: michi
 */

#include "../../../Edward.h"
#include "../../../MultiView.h"
#include "ModeModelMesh.h"
#include "ModeModelMeshTexture.h"

ModeModelMeshTexture *mode_model_mesh_texture = NULL;

ModeModelMeshTexture::ModeModelMeshTexture(Mode *_parent, DataModel *_data)
{
	name = "ModelMeshTexture";
	parent = _parent;
	data = _data;
	menu = HuiCreateResourceMenu("menu_model");
	multi_view = ed->multi_view_2d;
	Subscribe(data);
	Subscribe(multi_view, "SelectionChange");
}



ModeModelMeshTexture::~ModeModelMeshTexture()
{
	// TODO Auto-generated destructor stub
}



void ModeModelMeshTexture::OnRightButtonUp()
{
}



void ModeModelMeshTexture::Start()
{
}



void ModeModelMeshTexture::End()
{
}



void ModeModelMeshTexture::OnLeftButtonDown()
{
}



void ModeModelMeshTexture::DrawWin(int win, irect dest)
{
}



void ModeModelMeshTexture::OnMiddleButtonDown()
{
}



void ModeModelMeshTexture::OnUpdateMenu()
{
}



void ModeModelMeshTexture::Draw()
{
}



void ModeModelMeshTexture::OnRightButtonDown()
{
}

void ModeModelMeshTexture::OnKeyUp()
{
}



void ModeModelMeshTexture::OnMiddleButtonUp()
{
}



void ModeModelMeshTexture::OnLeftButtonUp()
{
}



void ModeModelMeshTexture::OnCommand(const string & id)
{
}



void ModeModelMeshTexture::OnMouseMove()
{
}



void ModeModelMeshTexture::OnUpdate(Observable *o)
{
}



void ModeModelMeshTexture::OnKeyDown()
{
}


