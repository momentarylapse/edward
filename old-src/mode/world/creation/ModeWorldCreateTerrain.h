/*
 * ModeWorldCreateTerrain.h
 *
 *  Created on: 14.06.2012
 *      Author: michi
 */

#ifndef MODEWORLDCREATETERRAIN_H_
#define MODEWORLDCREATETERRAIN_H_

#include "../../ModeCreation.h"
#include "../../../lib/math/vec3.h"

class DataWorld;
class ModeWorld;

class ModeWorldCreateTerrain: public ModeCreation<ModeWorld, DataWorld> {
public:
	ModeWorldCreateTerrain(ModeWorld *_parent);

	void on_start() override;
	void on_end() override;

	void on_left_button_up() override;
	void on_mouse_move() override;
	void on_draw_win(MultiView::Window *win) override;

	vec3 pos;
	vec3 size;
	bool pos_chosen;
};

#endif /* MODEWORLDCREATETERRAIN_H_ */
