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
	message = _("new terrain...");
}

void ModeWorldCreateTerrain::on_start()
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

	dialog->set_float("height_factor", 100);
	dialog->set_int("num_x", 64);
	dialog->set_int("num_z", 64);
	dialog->set_float("terrain_x", multi_view->cam.radius);//1000);
	dialog->set_float("terrain_z", multi_view->cam.radius);//1000);
	onSizeChange();

}

void ModeWorldCreateTerrain::on_end()
{
	delete(dialog);
	dialog = NULL;
}

void ModeWorldCreateTerrain::onOk()
{
	vector size = vector(dialog->get_float("terrain_x"), 0, dialog->get_float("terrain_z"));
	vector pos = multi_view->cam.pos - size / 2;
	int num_x = dialog->get_int("num_x");
	int num_z = dialog->get_int("num_z");
	data->AddNewTerrain(pos, size, num_x, num_z);
	abort();
}

void ModeWorldCreateTerrain::onClose()
{
	abort();
}

void ModeWorldCreateTerrain::onSizeChange()
{
	dialog->set_float("pattern_x", dialog->get_float("terrain_x") / (float)dialog->get_int("num_x"));
	dialog->set_float("pattern_z", dialog->get_float("terrain_z") / (float)dialog->get_int("num_z"));
}

void ModeWorldCreateTerrain::on_draw_win(MultiView::Window *win)
{
	parent->on_draw_win(win);
}


