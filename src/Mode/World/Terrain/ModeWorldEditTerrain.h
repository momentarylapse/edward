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

	void onStart() override;
	void onEnd() override;

	void onMouseMove() override;
	void onLeftButtonDown() override;
	void onLeftButtonUp() override;
	void onCommand(const string &id) override;

	void onDrawWin(MultiView::Window *win) override;

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
