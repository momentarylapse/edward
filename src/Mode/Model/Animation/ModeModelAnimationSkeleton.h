/*
 * ModeModelAnimationSkeleton.h
 *
 *  Created on: 16.08.2012
 *      Author: michi
 */

#ifndef MODEMODELANIMATIONSKELETON_H_
#define MODEMODELANIMATIONSKELETON_H_

#include "../../Mode.h"
#include "../../../Data/Model/DataModel.h"

class DataModel;

class ModeModelAnimationSkeleton: public Mode<DataModel> {
public:
	ModeModelAnimationSkeleton(ModeBase *parent, MultiView::MultiView *mv);

	void on_start() override;
	void on_end() override;

	void on_command(const string &id) override;
	void on_update_menu() override;
	void on_set_multi_view() override;

	void on_draw_win(MultiView::Window *win) override;

	void chooseMouseFunction(int f);
	int mouse_action;

	void updateSelection();
	bool select_recursive;

	int &current_move;
	int &current_frame;

	void copy();
	void paste();

	Array<ModelFrame> temp;
};

extern ModeModelAnimationSkeleton *mode_model_animation_skeleton;

#endif /* MODEMODELANIMATIONSKELETON_H_ */
