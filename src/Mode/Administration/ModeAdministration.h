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

	void _new();
	bool open();

	void on_start() override;
	void on_end() override;

	void on_command(const string &id) override;

	void BasicSettings();
	void ExportGame();

	void create_project_dir(const Path &dir);

	AdministrationDialog *dialog;
};

#endif /* MODEADMINISTRATION_H_ */
