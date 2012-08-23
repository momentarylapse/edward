/*
 * ModeAdministration.cpp
 *
 *  Created on: 23.08.2012
 *      Author: michi
 */

#include "ModeAdministration.h"
#include "../../Data/Administration/DataAdministration.h"
#include "Dialog/AdministrationDialog.h"
#include "../../Edward.h"

ModeAdministration *mode_administration;

ModeAdministration::ModeAdministration():
	Mode("Administration", NULL, new DataAdministration, NULL, "menu_administration")
{
	data = (DataAdministration*)data_generic;
}

ModeAdministration::~ModeAdministration()
{
}

void ModeAdministration::OnStart()
{
	data->LoadDatabase();
	data->UpdateDatabase();
	dialog = new AdministrationDialog(ed, true, data);
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


