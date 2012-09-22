/*
 * TerrainPropertiesDialog.h
 *
 *  Created on: 12.06.2012
 *      Author: michi
 */

#ifndef TERRAINPROPERTIESDIALOG_H_
#define TERRAINPROPERTIESDIALOG_H_

#include "../../../lib/hui/hui.h"
#include "../../../Data/World/DataWorld.h"
#include "../../../Stuff/Observer.h"

class TerrainPropertiesDialog: public CHuiWindow, public Observer
{
public:
	TerrainPropertiesDialog(CHuiWindow *_parent, bool _allow_parent, DataWorld *_data, int _index);
	virtual ~TerrainPropertiesDialog();

	void LoadData();
	void ApplyData();

	void FillTextureList();

	void OnSaveAs();
	void OnTextures();
	void OnTexturesEdit();
	void OnTexturesSelect();
	void OnAddTextureLevel();
	void OnDeleteTextureLevel();
	void OnClearTextureLevel();
	void OnTextureMapComplete();
	void OnDefaultMaterial();
	void OnMaterialFind();
	void OnOk();
	void OnClose();

	void OnUpdate(Observable *o);

private:
	DataWorld *data;
	int index;

	WorldEditingTerrain temp;
};

#endif /* TERRAINPROPERTIESDIALOG_H_ */
