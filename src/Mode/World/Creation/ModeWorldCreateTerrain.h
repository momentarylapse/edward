/*
 * ModeWorldCreateTerrain.h
 *
 *  Created on: 14.06.2012
 *      Author: michi
 */

#ifndef MODEWORLDCREATETERRAIN_H_
#define MODEWORLDCREATETERRAIN_H_

#include "../../ModeCreation.h"
#include "../../../Data/World/DataWorld.h"

class ModeWorldCreateTerrain: public ModeCreation, public HuiEventHandler
{
public:
	ModeWorldCreateTerrain(Mode *_parent);
	virtual ~ModeWorldCreateTerrain();

	void OnStart();
	void OnEnd();

	void OnDrawWin(int win, irect dest);

	void OnOk();
	void OnClose();
	void OnSizeChange();

	DataWorld *data;
	Data *GetData(){	return data;	}
};

#endif /* MODEWORLDCREATETERRAIN_H_ */
