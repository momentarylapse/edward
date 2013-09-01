/*
 * ModeWorldEditTerrain.cpp
 *
 *  Created on: 31.08.2013
 *      Author: michi
 */

#include "ModeWorldEditTerrain.h"

ModeWorldEditTerrain::ModeWorldEditTerrain(ModeBase* _parent, int _index) :
	ModeCreation("WorldEditTerrain", _parent)
{
	index = _index;
	terrain = &data->Terrains[index];
	message = _("malen...");
}

ModeWorldEditTerrain::~ModeWorldEditTerrain()
{
}

void ModeWorldEditTerrain::OnStart()
{
	multi_view->allow_mouse_actions = false;
}

void ModeWorldEditTerrain::OnEnd()
{
	multi_view->allow_mouse_actions = true;
}

void ModeWorldEditTerrain::OnMouseMove()
{
}

void ModeWorldEditTerrain::OnLeftButtonDown()
{
}

void ModeWorldEditTerrain::OnLeftButtonUp()
{
}

void ModeWorldEditTerrain::OnCommand(const string& id)
{
}

void ModeWorldEditTerrain::OnDrawWin(MultiViewWindow* win)
{
}
