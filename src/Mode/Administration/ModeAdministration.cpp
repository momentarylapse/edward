/*
 * ModeAdministration.cpp
 *
 *  Created on: 23.08.2012
 *      Author: michi
 */

#include "ModeAdministration.h"

ModeAdministration *mode_administration;

ModeAdministration::ModeAdministration():
	Mode("Administration", NULL, NULL, NULL, "menu_administration")
{
}

ModeAdministration::~ModeAdministration()
{
}

void ModeAdministration::OnStart()
{
}

void ModeAdministration::OnEnd()
{
}

void ModeAdministration::OnCommand(const string& id)
{
}

void ModeAdministration::OnUpdate(Observable* o)
{
}


