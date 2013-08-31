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

	virtual void OnStart();
	virtual void OnEnd();

	virtual void OnLeftButtonDown();
	virtual void OnLeftButtonUp();
	virtual void OnMouseMove();

	void OnDiameterSlider();
	void OnDepthSlider();

	virtual void OnDrawWin(MultiViewWindow *win);

	Action *GetAction();
	void Apply();

private:
	float base_diameter, base_depth;
	bool brushing;
	float distance;
	vector last_pos;
};

#endif /* MODEMODELMESHBRUSH_H_ */
