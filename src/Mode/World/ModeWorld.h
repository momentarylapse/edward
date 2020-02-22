/*
 * ModeWorld.h
 *
 *  Created on: 08.03.2012
 *      Author: michi
 */

#ifndef MODEWORLD_H_
#define MODEWORLD_H_

#include "../Mode.h"
#include "../../Data/World/DataWorld.h"

class WorldPropertiesDialog;
class WorldObjectListPanel;

class ModeWorld: public Mode<DataWorld> {
public:
	ModeWorld();
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


	void _new();
	bool open();
	bool save();
	bool save_as();

	bool optimize_view() override;

	void import_world_properties();
	void load_terrain();
	void set_ego();
	void apply_heightmap();

	void ExecutePropertiesDialog();
	void ExecuteWorldPropertiesDialog();
	void ExecuteTerrainPropertiesDialog(int index);
	void ExecuteLightmapDialog();
	WorldPropertiesDialog *world_dialog;
	WorldObjectListPanel *dialog;

	bool show_effects;
	int TerrainShowTextureLevel;

	void toggle_show_effects();

	void set_mouse_action(int mode);
	int mouse_action;

	Material temp_material;

	// clip board
	void copy();
	void paste();
	bool copyable();
	bool pasteable();

	Array<WorldObject> temp_objects;
	Array<WorldTerrain> temp_terrains;
};

extern ModeWorld* mode_world;

#endif /* MODEWORLD_H_ */
