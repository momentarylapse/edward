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

class ModeWorldCreateObject: public ModeCreation<DataWorld> {
public:
	ModeWorldCreateObject(ModeBase *_parent);

	void on_start() override;
	void on_end() override;

	void on_find_object();

	void on_left_button_down() override;

	string filename;
};

#endif /* MODEWORLDCREATEOBJECT_H_ */
