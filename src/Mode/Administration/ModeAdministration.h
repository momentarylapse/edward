/*
 * ModeAdministration.h
 *
 *  Created on: 23.08.2012
 *      Author: michi
 */

#ifndef MODEADMINISTRATION_H_
#define MODEADMINISTRATION_H_

#include "../Mode.h"

class ModeAdministration: public Mode {
public:
	ModeAdministration();
	virtual ~ModeAdministration();

	void OnStart();
	void OnEnd();

	void OnCommand(const string &id);
	void OnUpdate(Observable *o);
};

extern ModeAdministration *mode_administration;

#endif /* MODEADMINISTRATION_H_ */
