/*
 * ModeWorldEditTerrain.h
 *
 *  Created on: 31.08.2013
 *      Author: michi
 */

#ifndef MODEWORLDEDITTERRAIN_H_
#define MODEWORLDEDITTERRAIN_H_


#include "../../ModeCreation.h"
#include "../../../Data/World/DataWorld.h"

class ModeWorldEditTerrain: public ModeCreation<DataWorld>
{
public:
	ModeWorldEditTerrain(ModeBase *_parent, int index);
	virtual ~ModeWorldEditTerrain();

	virtual void OnStart();
	virtual void OnEnd();

	virtual void OnMouseMove();
	virtual void OnLeftButtonDown();
	virtual void OnLeftButtonUp();
	virtual void OnCommand(const string &id);

	virtual void OnDrawWin(MultiViewWindow *win);

	WorldTerrain *terrain;
	int index;
};

#endif /* MODEWORLDEDITTERRAIN_H_ */
