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

class ModeWorldCreateObject: public ModeCreation<DataWorld>
{
public:
	ModeWorldCreateObject(ModeBase *_parent);
	virtual ~ModeWorldCreateObject();

	virtual void on_start();
	virtual void on_end();

	void onFindObject();

	void on_left_button_down();

	string filename;
};

#endif /* MODEWORLDCREATEOBJECT_H_ */
