/*
 * ModeModelAnimationInterpolateFrames.h
 *
 *  Created on: 10.01.2015
 *      Author: michi
 */

#ifndef SRC_MODE_MODEL_ANIMATION_CREATION_MODEMODELANIMATIONINTERPOLATEFRAMES_H_
#define SRC_MODE_MODEL_ANIMATION_CREATION_MODEMODELANIMATIONINTERPOLATEFRAMES_H_


#include "../../../ModeCreation.h"
#include "../../../../data/model/DataModel.h"
#include "../../dialog/ModelAnimationTimelinePanel.h"

class ModeModelAnimation;

// FIXME .... this now has the wrong parent :(

class ModeModelAnimationInterpolateFrames: public ModeCreation<ModeModelAnimation, DataModel>, public TimeLineParasite {
public:
	ModeModelAnimationInterpolateFrames(ModeModelAnimation *_parent);

	virtual void on_start();
	virtual void on_end();

	virtual void on_draw_win(MultiView::Window *win);

	virtual void on_timeline_mouse_move();
	virtual void on_timeline_left_button_up();
	virtual void on_timeline_draw(Painter *p);

private:
};

#endif /* SRC_MODE_MODEL_ANIMATION_CREATION_MODEMODELANIMATIONINTERPOLATEFRAMES_H_ */
