/*
 * WorldTerrain.h
 *
 *  Created on: 08.03.2020
 *      Author: michi
 */

#ifndef SRC_DATA_WORLD_WORLDTERRAIN_H_
#define SRC_DATA_WORLD_WORLDTERRAIN_H_

#include "../../multiview/SingleData.h"
#include <y/world/Material.h>

class Terrain;
class ScriptInstanceData;
class Session;

/*class WorldTerrainVertex: public MultiView::SingleData
{
public:
	float Height;
	vector Normal;
};*/

struct Box {
	vec3 min, max;
};

class WorldTerrain: public MultiView::SingleData {
public:
	~WorldTerrain() override;

	Path filename;
	//Array<WorldTerrainVertex> Vertex;
	int VertexBufferSingle;
	int ShowTexture;
	bool Changed;
	Terrain *terrain;

	Array<ScriptInstanceData> components;

	bool load(Session *s, const Path &filename, bool deep = true);
	bool save(const Path &filename);

#if HAS_LIB_GL
	float hover_distance(MultiView::Window *win, const vec2 &m, vec3 &tp, float &z) override;
	bool in_rect(MultiView::Window *win, const rect &r) override;
	bool overlap_rect(MultiView::Window *win, const rect &r) override;
#endif

	Box bounding_box() const;

	void update_data();
};

class WorldEditingTerrain {
public:
	Path filename;
	//int num_x, num_z; // FIXME
	vec3 pattern;
	Path material_file;
	int num_textures;
	Path texture_file[MATERIAL_MAX_TEXTURES];
	vec3 texture_scale[MATERIAL_MAX_TEXTURES];
};



#endif /* SRC_DATA_WORLD_WORLDTERRAIN_H_ */
