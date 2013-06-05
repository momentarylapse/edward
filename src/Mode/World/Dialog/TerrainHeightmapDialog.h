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

class TerrainHeightmapDialog: public HuiWindow, public Observer
{
public:
	TerrainHeightmapDialog(HuiWindow *_parent, bool _allow_parent, DataWorld *_data);
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
	string heightmap_file;
	string filter_file;
	Image heightmap;
	Image filter;
	float stretch_x;
	float stretch_z;
};

#endif /* TERRAINHEIGHTMAPDIALOG_H_ */
