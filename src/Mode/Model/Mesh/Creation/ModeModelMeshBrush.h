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

class ModeModelMeshBrush: public ModeCreation<DataModel>
{
public:
	ModeModelMeshBrush(ModeBase *parent);

	void onStart() override;
	void onEnd() override;

	void onLeftButtonDown() override;
	void onLeftButtonUp() override;
	void onMouseMove() override;

	void onDiameterSlider();
	void onDepthSlider();

	void onDrawWin(MultiView::Window *win) override;

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
