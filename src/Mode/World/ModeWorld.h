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

class Mode;
class DataWorld;

class ModeWorld: public Mode
{
public:
	ModeWorld();
	virtual ~ModeWorld();

	void OnStart();
	void OnEnd();

	void OnMouseMove();
	void OnLeftButtonDown();
	void OnLeftButtonUp();
	void OnMiddleButtonDown();
	void OnMiddleButtonUp();
	void OnRightButtonDown();
	void OnRightButtonUp();
	void OnKeyDown();
	void OnKeyUp();
	void OnCommand(const string &id);
	void OnUpdate(Observable *o);
	void OnUpdateMenu();

	void OnDraw();
	void OnDrawWin(int win);


	void New();
	bool Open();
	bool Save();
	bool SaveAs();

	void OptimizeView();

	void ImportWorldProperties();
	void LoadTerrain();
	void SetEgo();
	void ApplyHeightmap();

	void ExecutePropertiesDialog();
	void ExecuteSelectionPropertiesDialog();
	void ExecuteWorldPropertiesDialog();
	void ExecuteObjectPropertiesDialog(int index);
	void ExecuteTerrainPropertiesDialog(int index);
	WorldPropertiesDialog *WorldDialog;

	DataWorld *data;

	int ViewStage;
	bool ShowTerrains;
	bool ShowObjects;
	bool ShowEffects;
	int TerrainShowTextureLevel;

	void ToggleShowTerrains();
	void ToggleShowObjects();
	void ToggleShowEffects();



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
