/*
 * ModeModelAnimationNone.h
 *
 *  Created on: 18.08.2012
 *      Author: michi
 */

#ifndef MODEMODELANIMATIONNONE_H_
#define MODEMODELANIMATIONNONE_H_

#include "../../Mode.h"
#include "../../../data/model/DataModel.h"

class DataModel;
class ModeModelAnimation;

class ModeModelAnimationNone: public Mode<ModeModelAnimation, DataModel> {
public:
	ModeModelAnimationNone(ModeModelAnimation *parent, MultiView::MultiView *mv);

	void on_start() override;
	void on_end() override;

	void on_draw_win(MultiView::Window *win) override;
};

#endif /* MODEMODELANIMATIONNONE_H_ */
