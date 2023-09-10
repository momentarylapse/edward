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
class ModeWorld;

class ModeWorldCreateObject: public ModeCreation<ModeWorld, DataWorld> {
public:
	ModeWorldCreateObject(ModeWorld *_parent);

	void on_start() override;
	void on_end() override;

	void on_find_object();

	void on_left_button_down() override;

	Path filename;
};

#endif /* MODEWORLDCREATEOBJECT_H_ */
