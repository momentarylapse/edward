/*
 * ModeWorldCreateTerrain.cpp
 *
 *  Created on: 14.06.2012
 *      Author: michi
 */

#include "ModeWorldCreateTerrain.h"
#include "../../../Data/World/DataWorld.h"
#include "../../../Edward.h"
#include "../../../MultiView/MultiView.h"

ModeWorldCreateTerrain::ModeWorldCreateTerrain(ModeBase *_parent) :
	ModeCreation<DataWorld>("WorldCreateTerrain", _parent)
{
	message = _("neues Terrain...");
}

ModeWorldCreateTerrain::~ModeWorldCreateTerrain()
{
}

void ModeWorldCreateTerrain::OnStart()
{
	// Dialog
	dialog = HuiCreateResourceDialog("new_terrain_dialog", ed);
	dialog->show();

	dialog->event("cancel", this, &ModeWorldCreateTerrain::OnClose);
	dialog->event("hui:close", this, &ModeWorldCreateTerrain::OnClose);
	dialog->event("ok", this, &ModeWorldCreateTerrain::OnOk);

	//dialog->event("height_image_find", this, &ModeWorldCreateTerrain::OnFindHeightmap);
	dialog->event("num_x", this, &ModeWorldCreateTerrain::OnSizeChange);
	dialog->event("num_z", this, &ModeWorldCreateTerrain::OnSizeChange);
	dialog->event("terrain_x", this, &ModeWorldCreateTerrain::OnSizeChange);
	dialog->event("terrain_z", this, &ModeWorldCreateTerrain::OnSizeChange);

	dialog->setFloat("height_factor", 100);
	dialog->setInt("num_x", 64);
	dialog->setInt("num_z", 64);
	dialog->setFloat("terrain_x", multi_view->cam.radius);//1000);
	dialog->setFloat("terrain_z", multi_view->cam.radius);//1000);
	OnSizeChange();

}

void ModeWorldCreateTerrain::OnEnd()
{
	delete(dialog);
	dialog = NULL;
}

void ModeWorldCreateTerrain::OnOk()
{
	vector size = vector(dialog->getFloat("terrain_x"), 0, dialog->getFloat("terrain_z"));
	vector pos = multi_view->cam.pos - size / 2;
	int num_x = dialog->getInt("num_x");
	int num_z = dialog->getInt("num_z");
	data->AddNewTerrain(pos, size, num_x, num_z);
	Abort();
}

void ModeWorldCreateTerrain::OnClose()
{
	Abort();
}

void ModeWorldCreateTerrain::OnSizeChange()
{
	dialog->setFloat("pattern_x", dialog->getFloat("terrain_x") / (float)dialog->getInt("num_x"));
	dialog->setFloat("pattern_z", dialog->getFloat("terrain_z") / (float)dialog->getInt("num_z"));
}

void ModeWorldCreateTerrain::OnDrawWin(MultiView::Window *win)
{
}


