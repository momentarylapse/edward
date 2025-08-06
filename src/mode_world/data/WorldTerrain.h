/*
 * WorldTerrain.h
 *
 *  Created on: 08.03.2020
 *      Author: michi
 */

#ifndef SRC_DATA_WORLD_WORLDTERRAIN_H_
#define SRC_DATA_WORLD_WORLDTERRAIN_H_

#include <view/SingleData.h>
#include <lib/yrenderer/Material.h>
#include <lib/math/Box.h>

class Terrain;
class ScriptInstanceData;
class Session;

/*class WorldTerrainVertex: public MultiView::SingleData
{
public:
	float Height;
	vector Normal;
};*/

struct WorldTerrain {

	Path filename;
	//Array<WorldTerrainVertex> Vertex;
	int VertexBufferSingle = -1;
	int ShowTexture = -1;
	bool Changed = false;
	Terrain *terrain = nullptr;

	bool load(Session *s, const Path &filename, bool deep = true);
	bool save(const Path &filename);

	/*float hover_distance(multiview::Window *win, const vec2 &m, vec3 &tp, float &z) override;
	bool in_rect(multiview::Window *win, const rect &r) override;
	bool overlap_rect(multiview::Window *win, const rect &r) override;*/


	Box bounding_box() const;
	void save_material(Session* s, const Path& filename);

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
