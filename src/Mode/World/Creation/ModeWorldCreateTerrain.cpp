/*
 * ModeWorldCreateTerrain.cpp
 *
 *  Created on: 14.06.2012
 *      Author: michi
 */

#include "ModeWorldCreateTerrain.h"
#include "../../../Edward.h"

ModeWorldCreateTerrain::ModeWorldCreateTerrain(Mode *_parent) :
	ModeCreation(_parent)
{
	name = "WorldCreateTerrain";
	data = (DataWorld*)_parent->GetData();

	message = _("neues Terrain...");
}

ModeWorldCreateTerrain::~ModeWorldCreateTerrain()
{
}

void ModeWorldCreateTerrain::OnStart()
{
	// Dialog
	dialog = HuiCreateResourceDialog("new_terrain_dialog", ed);
	dialog->Update();

	dialog->EventM("cancel", this, (void(HuiEventHandler::*)())&ModeWorldCreateTerrain::OnClose);
	dialog->EventM("hui:close", this, (void(HuiEventHandler::*)())&ModeWorldCreateTerrain::OnClose);
	dialog->EventM("ok", this, (void(HuiEventHandler::*)())&ModeWorldCreateTerrain::OnOk);

	//dialog->EventM("height_image_find", this, (void(HuiEventHandler::*)())&ModeWorldCreateTerrain::OnFindHeightmap);
	dialog->EventM("num_x", this, (void(HuiEventHandler::*)())&ModeWorldCreateTerrain::OnSizeChange);
	dialog->EventM("num_z", this, (void(HuiEventHandler::*)())&ModeWorldCreateTerrain::OnSizeChange);
	dialog->EventM("terrain_x", this, (void(HuiEventHandler::*)())&ModeWorldCreateTerrain::OnSizeChange);
	dialog->EventM("terrain_z", this, (void(HuiEventHandler::*)())&ModeWorldCreateTerrain::OnSizeChange);

	dialog->SetFloat("height_factor", 100);
	dialog->SetInt("num_x", 64);
	dialog->SetInt("num_z", 64);
	dialog->SetFloat("terrain_x", 1000);
	dialog->SetFloat("terrain_z", 1000);
	OnSizeChange();

}

void ModeWorldCreateTerrain::OnEnd()
{
	delete(dialog);
	dialog = NULL;
}

void ModeWorldCreateTerrain::OnOk()
{
	Abort();
}

void ModeWorldCreateTerrain::OnClose()
{
	Abort();
}

void ModeWorldCreateTerrain::OnSizeChange()
{
	dialog->SetFloat("pattern_x", dialog->GetFloat("terrain_x") / (float)dialog->GetInt("num_x"));
	dialog->SetFloat("pattern_z", dialog->GetFloat("terrain_z") / (float)dialog->GetInt("num_z"));
}

void ModeWorldCreateTerrain::OnDrawWin(int win, irect dest)
{
}


