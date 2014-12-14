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

	virtual void onStart();
	virtual void onEnd();

	virtual void onCommand(const string &id);
	virtual void onUpdate(Observable *o, const string &message);

	void BasicSettings();
	void ExportGame();

	AdministrationDialog *dialog;
};

extern ModeAdministration *mode_administration;

#endif /* MODEADMINISTRATION_H_ */
