/*
 * WorldTerrain.h
 *
 *  Created on: 08.03.2020
 *      Author: michi
 */

#ifndef SRC_DATA_WORLD_WORLDTERRAIN_H_
#define SRC_DATA_WORLD_WORLDTERRAIN_H_

#include "../../MultiView/SingleData.h"
#include "../../y/Material.h"

class Terrain;
class ScriptInstanceData;

/*class WorldTerrainVertex: public MultiView::SingleData
{
public:
	float Height;
	vector Normal;
};*/

class WorldTerrain: public MultiView::SingleData {
public:
	Path filename;
	//Array<WorldTerrainVertex> Vertex;
	int VertexBufferSingle;
	int ShowTexture;
	bool Changed;
	Terrain *terrain;

	Array<ScriptInstanceData> components;

	bool load(const vector &pos, const Path &filename, bool deep = true);
	bool save(const Path &filename);

	float hover_distance(MultiView::Window *win, const vector &m, vector &tp, float &z) override;
	bool in_rect(MultiView::Window *win, const rect &r) override;
	bool overlap_rect(MultiView::Window *win, const rect &r) override;

	void update_data();
};

class WorldEditingTerrain {
public:
	Path filename;
	int num_x, num_z;
	vector pattern;
	Path material_file;
	int num_textures;
	Path texture_file[MATERIAL_MAX_TEXTURES];
	vector texture_scale[MATERIAL_MAX_TEXTURES];
};



#endif /* SRC_DATA_WORLD_WORLDTERRAIN_H_ */
