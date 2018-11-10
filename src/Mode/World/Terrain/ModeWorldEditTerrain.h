/*
 * ModeWorldEditTerrain.h
 *
 *  Created on: 31.08.2013
 *      Author: michi
 */

#ifndef MODEWORLDEDITTERRAIN_H_
#define MODEWORLDEDITTERRAIN_H_


#include "../../ModeCreation.h"
#include "../../../Data/World/DataWorld.h"
#include "../../../lib/hui/hui.h"

class ModeWorldEditTerrain: public ModeCreation<DataWorld>, public hui::EventHandler
{
public:
	ModeWorldEditTerrain(ModeBase *_parent);
	virtual ~ModeWorldEditTerrain();

	void on_start() override;
	void on_end() override;

	void on_mouse_move() override;
	void on_left_button_down() override;
	void on_left_button_up() override;
	void on_command(const string &id) override;

	void on_draw_win(MultiView::Window *win) override;

	Action *getAction(const vector &pos);
	void apply(const vector &pos);

	void onDepthSlider();
	void onDiameterSlider();

private:
	float base_diameter, base_depth;
	bool brushing;
	vector last_pos;
};

#endif /* MODEWORLDEDITTERRAIN_H_ */
