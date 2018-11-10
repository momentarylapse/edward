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
#include "Dialog/WorldPropertiesDialog.h"

class ModeWorld: public Mode<DataWorld>
{
public:
	ModeWorld();
	virtual ~ModeWorld();

	void on_start() override;
	void on_end() override;

	void on_command(const string &id) override;
	void on_update(Observable *o, const string &message) override;
	void on_update_menu() override;
	void on_set_multi_view() override;

	void on_draw() override;
	void on_draw_win(MultiView::Window *win) override;


	void _new() override;
	bool open() override;
	bool save() override;
	bool save_as() override;

	bool optimize_view() override;

	void ImportWorldProperties();
	void LoadTerrain();
	void SetEgo();
	void ApplyHeightmap();

	void ExecutePropertiesDialog();
	void ExecuteSelectionPropertiesDialog();
	void ExecuteWorldPropertiesDialog();
	void ExecuteObjectPropertiesDialog(int index);
	void ExecuteTerrainPropertiesDialog(int index);
	void ExecuteLightmapDialog();
	WorldPropertiesDialog *WorldDialog;

	bool ShowTerrains;
	bool ShowObjects;
	bool ShowEffects;
	int TerrainShowTextureLevel;

	void ToggleShowTerrains();
	void ToggleShowObjects();
	void ToggleShowEffects();

	void SetMouseAction(int mode);
	int mouse_action;

	Material temp_material;

	// clip board
	void Copy();
	void Paste();
	bool Copyable();
	bool Pasteable();

	Array<WorldObject> temp_objects;
	Array<WorldTerrain> temp_terrains;
};

extern ModeWorld* mode_world;

#endif /* MODEWORLD_H_ */
