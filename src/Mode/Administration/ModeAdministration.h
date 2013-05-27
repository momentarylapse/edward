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

	virtual void OnStart();
	virtual void OnEnd();

	virtual void OnCommand(const string &id);
	virtual void OnUpdate(Observable *o);

	void BasicSettings();
	void ExportGame();

	AdministrationDialog *dialog;
};

extern ModeAdministration *mode_administration;

#endif /* MODEADMINISTRATION_H_ */
