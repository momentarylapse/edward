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

class ModeModelAnimationInterpolateFrames: public ModeCreation<DataModel>, public TimeLineParasite {
public:
	ModeModelAnimationInterpolateFrames(ModeBase *_parent);

	virtual void on_start();
	virtual void on_end();

	virtual void on_draw_win(MultiView::Window *win);

	virtual void on_timeline_mouse_move();
	virtual void on_timeline_left_button_up();
	virtual void on_timeline_draw(Painter *p);

private:
};

#endif /* SRC_MODE_MODEL_ANIMATION_CREATION_MODEMODELANIMATIONINTERPOLATEFRAMES_H_ */
