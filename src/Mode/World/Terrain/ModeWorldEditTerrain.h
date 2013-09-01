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
#include "../../../lib/hui/hui.h"

class ModeWorldEditTerrain: public ModeCreation<DataWorld>, public HuiEventHandler
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

	Action *GetAction();
	void Apply();

	void OnDepthSlider();
	void OnDiameterSlider();

private:
	WorldTerrain *terrain;
	int index;
	float base_diameter, base_depth;
	bool brushing;
	float distance;
	vector last_pos;
};

#endif /* MODEWORLDEDITTERRAIN_H_ */
