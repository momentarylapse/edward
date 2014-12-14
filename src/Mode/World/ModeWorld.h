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

	virtual void onStart();
	virtual void onEnd();

	virtual void onMouseMove();
	virtual void onLeftButtonDown();
	virtual void onLeftButtonUp();
	virtual void onMiddleButtonDown();
	virtual void onMiddleButtonUp();
	virtual void onRightButtonDown();
	virtual void onRightButtonUp();
	virtual void onKeyDown();
	virtual void onKeyUp();
	virtual void onCommand(const string &id);
	virtual void onUpdate(Observable *o, const string &message);
	virtual void onUpdateMenu();

	virtual void onDraw();
	virtual void onDrawWin(MultiView::Window *win);


	virtual void _new();
	virtual bool open();
	virtual bool save();
	virtual bool saveAs();

	virtual bool optimizeView();

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
