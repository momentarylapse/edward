/*
 * ModeWorldCreateLink.h
 *
 *  Created on: Mar 8, 2020
 *      Author: michi
 */

#ifndef SRC_MODE_WORLD_CREATION_MODEWORLDCREATELINK_H_
#define SRC_MODE_WORLD_CREATION_MODEWORLDCREATELINK_H_

#include "../../ModeCreation.h"
class DataWorld;

class ModeWorldCreateLink: public ModeCreation<DataWorld> {
public:
	ModeWorldCreateLink(ModeBase *_parent);

	void on_start() override;
	void on_end() override;

	void on_draw_win(MultiView::Window *win) override;

	void on_find_object();

	void on_left_button_down() override;

	Array<int> objects;
};

#endif /* SRC_MODE_WORLD_CREATION_MODEWORLDCREATELINK_H_ */
