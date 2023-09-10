/*
 * ModeModelSkeleton.h
 *
 *  Created on: 14.03.2012
 *      Author: michi
 */

#ifndef MODEMODELSKELETON_H_
#define MODEMODELSKELETON_H_

#include "../../Mode.h"
#include "../../../data/model/DataModel.h"

class DataModel;
class ModeModel;
class ModeModelMesh;

class ModeModelSkeleton: public Mode<ModeModel, DataModel> {
public:
	ModeModelSkeleton(ModeModel *_parent, MultiView::MultiView *mv);

	void on_start() override;
	void on_end() override;

	void on_command(const string &id) override;
	void on_update_menu() override;
	void on_set_multi_view() override;

	void on_draw() override;
	void on_draw_win(MultiView::Window *win) override;

	void add_sub_model();
	void remove_sub_model();
	void unlink_selection();

	void draw_skeleton(MultiView::Window *win, Array<ModelBone> &bone, bool thin = false);


	void choose_mouse_function(int f);
	int mouse_action;

	ModeModelMesh *mode_model_mesh;
};

#endif /* MODEMODELSKELETON_H_ */
