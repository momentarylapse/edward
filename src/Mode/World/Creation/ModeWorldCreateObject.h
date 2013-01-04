/*
 * ModeWorldCreateObject.h
 *
 *  Created on: 12.06.2012
 *      Author: michi
 */

#ifndef MODEWORLDCREATEOBJECT_H_
#define MODEWORLDCREATEOBJECT_H_

#include "../../ModeCreation.h"
#include "../../../lib/hui/hui.h"
class DataWorld;

class ModeWorldCreateObject: public ModeCreation, public HuiEventHandler
{
public:
	ModeWorldCreateObject(Mode *_parent);
	virtual ~ModeWorldCreateObject();

	void OnStart();
	void OnEnd();

	void OnFindObject();

	void OnLeftButtonDown();

	DataWorld *data;

	string filename;
};

#endif /* MODEWORLDCREATEOBJECT_H_ */
