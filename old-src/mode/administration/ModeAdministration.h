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

class ModeAdministration: public Mode<ModeAdministration, DataAdministration> {
public:
	ModeAdministration(Session *s);
	virtual ~ModeAdministration();

	void _new();
	bool open();

	void on_start() override;
	void on_end() override;

	void on_command(const string &id) override;

	void basic_settings();
	void export_game();

	void create_project(const Path &dir, const string &first_world);
	void upgrade_project(const Path &dir);

	AdministrationDialog *dialog;
};

#endif /* MODEADMINISTRATION_H_ */
