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

	void Start();
	void End();

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

	void Draw();
	void DrawWin(int win, irect dest);


	void New();
	bool Open();
	bool Save();
	bool SaveAs();

	void ExecutePropertiesSelectionDialog();
	void ExecuteWorldPropertiesDialog();
	void ExecuteObjectPropertiesDialog();
	void ExecuteTerrainPropertiesDialog();

	WorldPropertiesDialog *WorldDialog;

	DataWorld *data;

	int ViewStage;
	bool ShowTerrains;
	bool ShowObjects;
	int TerrainShowTextureLevel;
};

extern ModeWorld* mode_world;

#endif /* MODEWORLD_H_ */
