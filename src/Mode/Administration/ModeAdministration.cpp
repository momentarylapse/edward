/*
 * ModeAdministration.cpp
 *
 *  Created on: 23.08.2012
 *      Author: michi
 */

#include "ModeAdministration.h"
#include "Dialog/AdministrationDialog.h"
#include "../../Edward.h"

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
	dialog = new AdministrationDialog(ed, true);
	dialog->Update();
}

void ModeAdministration::OnEnd()
{
	delete(dialog);
}

void ModeAdministration::OnCommand(const string& id)
{
}

void ModeAdministration::OnUpdate(Observable* o)
{
}


