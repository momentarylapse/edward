/*
 * ModeAdministration.h
 *
 *  Created on: 23.08.2012
 *      Author: michi
 */

#ifndef MODEADMINISTRATION_H_
#define MODEADMINISTRATION_H_

#include "../Mode.h"
class DataAdministration;
class AdministrationDialog;

class ModeAdministration: public Mode<DataAdministration> {
public:
	ModeAdministration();
	virtual ~ModeAdministration();

	void _new() override;
	bool open() override;

	void onStart() override;
	void onEnd() override;

	void onCommand(const string &id) override;
	void onUpdate(Observable *o, const string &message) override;

	void BasicSettings();
	void ExportGame();

	void create_project_dir(const string &dir);

	AdministrationDialog *dialog;
};

extern ModeAdministration *mode_administration;

#endif /* MODEADMINISTRATION_H_ */
