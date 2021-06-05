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

	bool load(const vector &pos, const Path &filename, bool deep = true);
	bool save(const Path &filename);

	float hover_distance(MultiView::Window *win, const vector &m, vector &tp, float &z) override;
	bool in_rect(MultiView::Window *win, const rect &r) override;
	bool overlap_rect(MultiView::Window *win, const rect &r) override;

	void update_data();
};

class WorldEditingTerrain {
public:
	Path FileName;
	int NumX, NumZ;
	vector Pattern;
	Path MaterialFile;
	int NumTextures;
	Path TextureFile[MATERIAL_MAX_TEXTURES];
	vector TextureScale[MATERIAL_MAX_TEXTURES];
};



#endif /* SRC_DATA_WORLD_WORLDTERRAIN_H_ */
