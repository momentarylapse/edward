/*
 * ModeWorldCreateObject.h
 *
 *  Created on: 12.06.2012
 *      Author: michi
 */

#ifndef MODEWORLDCREATEOBJECT_H_
#define MODEWORLDCREATEOBJECT_H_

#include "../../ModeCreation.h"
class DataWorld;

class ModeWorldCreateObject: public ModeCreation
{
public:
	ModeWorldCreateObject(Mode *_parent);
	virtual ~ModeWorldCreateObject();

	void OnLeftButtonDown();

	DataWorld *data;

	string filename;
};

#endif /* MODEWORLDCREATEOBJECT_H_ */
