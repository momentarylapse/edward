/*
 * ModeModelAnimationInterpolateFrames.h
 *
 *  Created on: 10.01.2015
 *      Author: michi
 */

#ifndef SRC_MODE_MODEL_ANIMATION_CREATION_MODEMODELANIMATIONINTERPOLATEFRAMES_H_
#define SRC_MODE_MODEL_ANIMATION_CREATION_MODEMODELANIMATIONINTERPOLATEFRAMES_H_


#include "../../../ModeCreation.h"
#include "../../../../Data/Model/DataModel.h"
#include "../../Dialog/ModelAnimationTimelinePanel.h"

class ModeModelAnimationInterpolateFrames: public ModeCreation<DataModel>, public HuiEventHandler, public TimeLineParasite
{
public:
	ModeModelAnimationInterpolateFrames(ModeBase *_parent);
	virtual ~ModeModelAnimationInterpolateFrames();

	virtual void onStart();
	virtual void onEnd();

	virtual void onDrawWin(MultiView::Window *win);

	virtual void onTimelineMouseMove();
	virtual void onTimelineLeftButtonUp();
	virtual void onTimelineDraw(HuiPainter *p);

private:
};

#endif /* SRC_MODE_MODEL_ANIMATION_CREATION_MODEMODELANIMATIONINTERPOLATEFRAMES_H_ */
