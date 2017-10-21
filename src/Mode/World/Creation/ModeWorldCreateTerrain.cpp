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

void ModeWorldCreateTerrain::onStart()
{
	// Dialog
	dialog = hui::CreateResourceDialog("new_terrain_dialog", ed);
	dialog->show();

	dialog->event("cancel", std::bind(&ModeWorldCreateTerrain::onClose, this));
	dialog->event("hui:close", std::bind(&ModeWorldCreateTerrain::onClose, this));
	dialog->event("ok", std::bind(&ModeWorldCreateTerrain::onOk, this));

	//dialog->event("height_image_find", std::bind(&ModeWorldCreateTerrain::OnFindHeightmap, this));
	dialog->event("num_x", std::bind(&ModeWorldCreateTerrain::onSizeChange, this));
	dialog->event("num_z", std::bind(&ModeWorldCreateTerrain::onSizeChange, this));
	dialog->event("terrain_x", std::bind(&ModeWorldCreateTerrain::onSizeChange, this));
	dialog->event("terrain_z", std::bind(&ModeWorldCreateTerrain::onSizeChange, this));

	dialog->setFloat("height_factor", 100);
	dialog->setInt("num_x", 64);
	dialog->setInt("num_z", 64);
	dialog->setFloat("terrain_x", multi_view->cam.radius);//1000);
	dialog->setFloat("terrain_z", multi_view->cam.radius);//1000);
	onSizeChange();

}

void ModeWorldCreateTerrain::onEnd()
{
	delete(dialog);
	dialog = NULL;
}

void ModeWorldCreateTerrain::onOk()
{
	vector size = vector(dialog->getFloat("terrain_x"), 0, dialog->getFloat("terrain_z"));
	vector pos = multi_view->cam.pos - size / 2;
	int num_x = dialog->getInt("num_x");
	int num_z = dialog->getInt("num_z");
	data->AddNewTerrain(pos, size, num_x, num_z);
	abort();
}

void ModeWorldCreateTerrain::onClose()
{
	abort();
}

void ModeWorldCreateTerrain::onSizeChange()
{
	dialog->setFloat("pattern_x", dialog->getFloat("terrain_x") / (float)dialog->getInt("num_x"));
	dialog->setFloat("pattern_z", dialog->getFloat("terrain_z") / (float)dialog->getInt("num_z"));
}

void ModeWorldCreateTerrain::onDrawWin(MultiView::Window *win)
{
	parent->onDrawWin(win);
}


