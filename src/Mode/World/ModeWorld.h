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

	void onStart() override;
	void onEnd() override;

	void onCommand(const string &id) override;
	void onUpdate(Observable *o, const string &message) override;
	void onUpdateMenu() override;
	void onSetMultiView() override;

	void onDraw() override;
	void onDrawWin(MultiView::Window *win) override;


	void _new() override;
	bool open() override;
	bool save() override;
	bool saveAs() override;

	bool optimizeView() override;

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
