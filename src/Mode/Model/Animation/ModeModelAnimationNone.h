/*
 * ModeModelAnimationNone.h
 *
 *  Created on: 18.08.2012
 *      Author: michi
 */

#ifndef MODEMODELANIMATIONNONE_H_
#define MODEMODELANIMATIONNONE_H_

#include "../../Mode.h"
#include "../../../Data/Model/DataModel.h"

class DataModel;

class ModeModelAnimationNone: public Mode<DataModel> {
public:
	ModeModelAnimationNone(ModeBase *parent);

	void on_start() override;
	void on_end() override;

	void on_draw_win(MultiView::Window *win) override;
};

extern ModeModelAnimationNone *mode_model_animation_none;

#endif /* MODEMODELANIMATIONNONE_H_ */
