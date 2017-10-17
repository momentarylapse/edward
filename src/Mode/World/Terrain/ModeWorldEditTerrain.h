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

class ModeWorldEditTerrain: public ModeCreation<DataWorld>, public hui::EventHandler
{
public:
	ModeWorldEditTerrain(ModeBase *_parent);
	virtual ~ModeWorldEditTerrain();

	virtual void onStart();
	virtual void onEnd();

	virtual void onMouseMove();
	virtual void onLeftButtonDown();
	virtual void onLeftButtonUp();
	virtual void onCommand(const string &id);

	virtual void onDrawWin(MultiView::Window *win);

	Action *getAction(const vector &pos);
	void apply(const vector &pos);

	void onDepthSlider();
	void onDiameterSlider();

private:
	float base_diameter, base_depth;
	bool brushing;
	vector last_pos;
};

#endif /* MODEWORLDEDITTERRAIN_H_ */
