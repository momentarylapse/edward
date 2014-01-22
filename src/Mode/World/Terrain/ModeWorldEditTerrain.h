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
	ModeWorldEditTerrain(ModeBase *_parent);
	virtual ~ModeWorldEditTerrain();

	virtual void OnStart();
	virtual void OnEnd();

	virtual void OnMouseMove();
	virtual void OnLeftButtonDown();
	virtual void OnLeftButtonUp();
	virtual void OnCommand(const string &id);

	virtual void OnDrawWin(MultiView::Window *win);

	Action *GetAction(const vector &pos);
	void Apply(const vector &pos);

	void OnDepthSlider();
	void OnDiameterSlider();

private:
	float base_diameter, base_depth;
	bool brushing;
	vector last_pos;
};

#endif /* MODEWORLDEDITTERRAIN_H_ */
