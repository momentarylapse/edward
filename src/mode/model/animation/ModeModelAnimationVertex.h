/*
 * ModeModelAnimationVertex.h
 *
 *  Created on: 16.08.2012
 *      Author: michi
 */

#ifndef MODEMODELANIMATIONVERTEX_H_
#define MODEMODELANIMATIONVERTEX_H_

#include "../../Mode.h"
#include "../../../data/model/DataModel.h"

class DataModel;
class ModeModelAnimation;

class ModeModelAnimationVertex: public Mode<ModeModelAnimation, DataModel> {
public:
	ModeModelAnimationVertex(ModeModelAnimation *parent, MultiView::MultiView *mv);

	void on_start() override;
	void on_end() override;

	void on_command(const string &id) override;
	void on_data_change();
	void on_update_menu() override;

	void on_draw_win(MultiView::Window *win) override;

	void chooseMouseFunction(int f);

	int mouse_action;
};

extern ModeModelAnimationVertex *mode_model_animation_vertex;

#endif /* MODEMODELANIMATIONVERTEX_H_ */
