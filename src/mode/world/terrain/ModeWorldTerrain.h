/*
 * ModeWorldEditTerrain.h
 *
 *  Created on: 31.08.2013
 *      Author: michi
 */

#ifndef MODEWORLDEDITTERRAIN_H_
#define MODEWORLDEDITTERRAIN_H_


#include "../../ModeCreation.h"
#include "../../../data/world/DataWorld.h"
#include "../../../lib/hui/hui.h"

class TerrainDeformBrushPanel;
class ModeWorld;

class ModeWorldTerrain: public ModeCreation<ModeWorld, DataWorld> {
public:
	ModeWorldTerrain(ModeWorld *_parent);

	void on_start() override;
	void on_end() override;

	void on_mouse_move() override;
	void on_left_button_down() override;
	void on_left_button_up() override;
	void on_command(const string &id) override;

	void on_draw_win(MultiView::Window *win) override;

	Action *get_action(const vec3 &pos);
	void apply(const vec3 &pos);

	TerrainDeformBrushPanel *dialog;

private:
	float base_diameter, base_depth;
	bool brushing;
	vec3 last_pos;
};

extern ModeWorldTerrain *mode_world_terrain;

#endif /* MODEWORLDEDITTERRAIN_H_ */
