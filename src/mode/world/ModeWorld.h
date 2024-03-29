/*
 * ModeWorld.h
 *
 *  Created on: 08.03.2012
 *      Author: michi
 */

#ifndef MODEWORLD_H_
#define MODEWORLD_H_

#include "../Mode.h"
#include "../../data/world/DataWorld.h"

class WorldPropertiesDialog;
class WorldObjectListPanel;

class ModeWorld: public Mode<ModeWorld, DataWorld> {
public:
	ModeWorld(Session *s, MultiView::MultiView *mv);
	virtual ~ModeWorld();

	void on_start() override;
	void on_end() override;
	void on_enter() override;
	void on_leave() override;

	void on_command(const string &id) override;
	void on_update_menu() override;
	void on_set_multi_view() override;

	void on_draw() override;
	void on_draw_win(MultiView::Window *win) override;

	void draw_background(MultiView::Window *win);
	void draw_terrains(MultiView::Window *win);
	void draw_objects(MultiView::Window *win);
	void draw_cameras(MultiView::Window *win);
	void draw_lights(MultiView::Window *win);
	void draw_links(MultiView::Window *win);


	void _new();
	void open();
	void save();
	void save_as();

	bool optimize_view() override;

	void import_world_properties();
	void load_terrain();
	void set_ego();
	void apply_heightmap();

	void apply_lighting(MultiView::Window *win);

	void ExecuteWorldPropertiesDialog();
	void ExecuteTerrainPropertiesDialog(int index);
	void ExecuteLightmapDialog();
	WorldPropertiesDialog *world_dialog;
	shared<WorldObjectListPanel> dialog;

	bool show_effects;
	int TerrainShowTextureLevel;

	void toggle_show_effects();

	void set_mouse_action(int mode);
	int mouse_action;

	owned<Material> temp_material;

	// clip board
	void copy();
	void paste();
	bool copyable();
	bool pasteable();

	Array<WorldObject> temp_objects;
	Array<WorldTerrain> temp_terrains;
	Array<WorldCamera> temp_cameras;
	Array<WorldLight> temp_lights;
};

#endif /* MODEWORLD_H_ */
