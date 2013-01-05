/*
 * ModeWorldCreateTerrain.h
 *
 *  Created on: 14.06.2012
 *      Author: michi
 */

#ifndef MODEWORLDCREATETERRAIN_H_
#define MODEWORLDCREATETERRAIN_H_

#include "../../ModeCreation.h"
class DataWorld;

class ModeWorldCreateTerrain: public ModeCreation, public HuiEventHandler
{
public:
	ModeWorldCreateTerrain(Mode *_parent);
	virtual ~ModeWorldCreateTerrain();

	void OnStart();
	void OnEnd();

	void OnDrawWin(int win);

	void OnOk();
	void OnClose();
	void OnSizeChange();

	DataWorld *data;
};

#endif /* MODEWORLDCREATETERRAIN_H_ */
