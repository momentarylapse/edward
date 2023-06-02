/*
 * ModeWorldCreateLight.h
 *
 *  Created on: Mar 8, 2020
 *      Author: michi
 */

#ifndef SRC_MODE_WORLD_CREATION_MODEWORLDCREATELIGHT_H_
#define SRC_MODE_WORLD_CREATION_MODEWORLDCREATELIGHT_H_

#include "../../ModeCreation.h"
class DataWorld;

class ModeWorldCreateLight: public ModeCreation<DataWorld> {
public:
	ModeWorldCreateLight(ModeBase *_parent);

	void on_start() override;
	void on_end() override;

	void on_draw_win(MultiView::Window *win) override;

	void on_left_button_down() override;
};

#endif /* SRC_MODE_WORLD_CREATION_MODEWORLDCREATELIGHT_H_ */
