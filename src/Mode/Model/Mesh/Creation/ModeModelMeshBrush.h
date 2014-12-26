/*
 * ModeModelMeshBrush.h
 *
 *  Created on: 22.08.2013
 *      Author: michi
 */

#ifndef MODEMODELMESHBRUSH_H_
#define MODEMODELMESHBRUSH_H_

#include "../../../ModeCreation.h"
#include "../../../../Data/Model/DataModel.h"
#include "../../../../lib/hui/hui.h"

class ModeModelMeshBrush: public ModeCreation<DataModel>, public HuiEventHandler
{
public:
	ModeModelMeshBrush(ModeBase *parent);
	virtual ~ModeModelMeshBrush();

	virtual void onStart();
	virtual void onEnd();

	virtual void onLeftButtonDown();
	virtual void onLeftButtonUp();
	virtual void onMouseMove();

	void onDiameterSlider();
	void onDepthSlider();

	virtual void onDrawWin(MultiView::Window *win);

	void onClose();

	Action *getAction();
	void apply();

private:
	float base_diameter, base_depth;
	bool brushing;
	float distance;
	vector last_pos;
};

#endif /* MODEMODELMESHBRUSH_H_ */
