/*
 * ModeMaterial.cpp
 *
 *  Created on: 08.03.2012
 *      Author: michi
 */

#include "../../Edward.h"
#include "ModeMaterial.h"
#include "../../Data/Material/DataMaterial.h"
#include "../../lib/x/x.h"

ModeMaterial *mode_material = NULL;

ModeMaterial::ModeMaterial()
{
	name = "Material";
	parent = NULL;

	menu = HuiCreateResourceMenu("menu_material");
	data = new DataMaterial;
	multi_view = ed->multi_view_3d;
}

ModeMaterial::~ModeMaterial()
{
}

void ModeMaterial::New()
{
	data->Reset();
	multi_view->Reset();
	multi_view->radius = 500;
	ed->SetMode(mode_material);
}



void ModeMaterial::OnKeyUp()
{
}



bool ModeMaterial::Save()
{
}



void ModeMaterial::Draw()
{
}



void ModeMaterial::OnDataChange()
{
}



void ModeMaterial::OnCommand(const string & id)
{
}



void ModeMaterial::OnRightButtonUp()
{
}



void ModeMaterial::DrawWin(int win, irect dest)
{
	FxDrawBall(v0, 100, 16, 32);
}



void ModeMaterial::OnMouseMove()
{
}



bool ModeMaterial::Open()
{
	data->Reset();
	multi_view->Reset();
	ed->SetMode(mode_material);
}



void ModeMaterial::OnMiddleButtonDown()
{
}



void ModeMaterial::OnRightButtonDown()
{
}



void ModeMaterial::OnLeftButtonDown()
{
}



void ModeMaterial::OnLeftButtonUp()
{
}



void ModeMaterial::End()
{
}



bool ModeMaterial::SaveAs()
{
}



void ModeMaterial::OnKeyDown()
{
}



void ModeMaterial::Start()
{
}



void ModeMaterial::OnMiddleButtonUp()
{
}


