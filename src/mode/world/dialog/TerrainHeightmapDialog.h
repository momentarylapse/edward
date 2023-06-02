/*
 * TerrainHeightmapDialog.h
 *
 *  Created on: 17.06.2012
 *      Author: michi
 */

#ifndef TERRAINHEIGHTMAPDIALOG_H_
#define TERRAINHEIGHTMAPDIALOG_H_

#include "../../../lib/hui/hui.h"
#include "../../../data/world/DataWorld.h"

class TerrainHeightmapDialog: public hui::Dialog {
public:
	TerrainHeightmapDialog(hui::Window *_parent, bool _allow_parent, DataWorld *_data);

	void load_data();
	void apply_data();

	void on_find_heightmap();
	void on_find_filter();
	void on_size_change();
	void on_preview_draw(Painter *p);
	void on_ok();
	void on_close();

private:
	DataWorld *data;
	Path heightmap_file;
	Path filter_file;
	Image heightmap;
	Image filter;
	float stretch_x;
	float stretch_z;
};

#endif /* TERRAINHEIGHTMAPDIALOG_H_ */
