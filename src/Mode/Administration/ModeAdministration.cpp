/*
 * ModeAdministration.cpp
 *
 *  Created on: 23.08.2012
 *      Author: michi
 */

#include "ModeAdministration.h"
#include "../../Data/Administration/DataAdministration.h"
#include "Dialog/AdministrationDialog.h"
#include "Dialog/ConfigurationDialog.h"
#include "../../Edward.h"

ModeAdministration *mode_administration;

ModeAdministration::ModeAdministration():
	Mode<DataAdministration>("Administration", NULL, new DataAdministration, NULL, "menu_administration")
{
}

ModeAdministration::~ModeAdministration()
{
}

void ModeAdministration::onStart()
{
	data->LoadDatabase();
	data->UpdateDatabase();
	dialog = new AdministrationDialog(ed, true, data);
	dialog->show();
}

void ModeAdministration::onEnd()
{
	delete(dialog);
}

void ModeAdministration::onCommand(const string& id)
{
	if (id == "export_game")
		ExportGame();
	if (id == "rudimentary_configuration")
		BasicSettings();
}

void ModeAdministration::onUpdate(Observable* o)
{
}

void ModeAdministration::BasicSettings()
{
	ConfigurationDialog *dlg = new ConfigurationDialog(HuiCurWindow, false, data, false);
	dlg->run();
}

void ModeAdministration::ExportGame()
{
	ConfigurationDialog *dlg = new ConfigurationDialog(HuiCurWindow, false, data, true);
	dlg->run();

	// data->ExportGame(...);
}



