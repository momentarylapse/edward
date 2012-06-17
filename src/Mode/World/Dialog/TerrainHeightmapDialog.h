/*
 * TerrainHeightmapDialog.h
 *
 *  Created on: 17.06.2012
 *      Author: michi
 */

#ifndef TERRAINHEIGHTMAPDIALOG_H_
#define TERRAINHEIGHTMAPDIALOG_H_

#include "../../../lib/hui/hui.h"
#include "../../../Data/World/DataWorld.h"
#include "../../../Stuff/Observer.h"

class TerrainHeightmapDialog: public CHuiWindow, public Observer
{
public:
	TerrainHeightmapDialog(CHuiWindow *_parent, bool _allow_parent, DataWorld *_data);
	virtual ~TerrainHeightmapDialog();

	void LoadData();
	void ApplyData();

	void OnFindHeightmap();
	void OnFindFilter();
	void OnSizeChange();
	void OnPreviewDraw();
	void OnOk();
	void OnClose();

	void OnUpdate(Observable *o);

private:
	DataWorld *data;
	Image heightmap;
	Image filter;
};

#endif /* TERRAINHEIGHTMAPDIALOG_H_ */
