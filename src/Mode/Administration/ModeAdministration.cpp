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

void ModeAdministration::OnStart()
{
	data->LoadDatabase();
	data->UpdateDatabase();
	dialog = new AdministrationDialog(ed, true, data);
	dialog->Show();
}

void ModeAdministration::OnEnd()
{
	delete(dialog);
}

void ModeAdministration::OnCommand(const string& id)
{
	if (id == "export_game")
		ExportGame();
	if (id == "rudimentary_configuration")
		BasicSettings();
}

void ModeAdministration::OnUpdate(Observable* o)
{
}

void ModeAdministration::BasicSettings()
{
	ConfigurationDialog *dlg = new ConfigurationDialog(HuiCurWindow, false, data, false);
	dlg->Run();
}

void ModeAdministration::ExportGame()
{
	ConfigurationDialog *dlg = new ConfigurationDialog(HuiCurWindow, false, data, true);
	dlg->Run();

	// data->ExportGame(...);
}



