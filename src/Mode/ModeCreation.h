/*
 * ModeCreation.h
 *
 *  Created on: 07.03.2012
 *      Author: michi
 */

#ifndef MODECREATION_H_
#define MODECREATION_H_

#include "Mode.h"
#include "../lib/hui/hui.h"
#include "../lib/file/file.h"

class ModeCreation: public Mode
{
public:
	ModeCreation(Mode *_parent);
	virtual void OnDrawRecursive();

	virtual void Abort();
	string message;
	CHuiWindow *dialog;
};

#endif /* MODECREATION_H_ */
