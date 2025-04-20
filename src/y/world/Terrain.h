/*----------------------------------------------------------------------------*\
| Terrain                                                                      |
| -> terrain of a world                                                        |
|                                                                              |
| vital properties:                                                            |
|                                                                              |
| last updated: 2008.06.23 (c) by MichiSoft TM                                 |
\*----------------------------------------------------------------------------*/
#pragma once

#include <lib/base/base.h>
#include <lib/os/path.h>
#include <lib/math/vec3.h>
#include "../y/Component.h"
#include "Material.h"
#include "../graphics-fwd.h"
#include "../y/BaseClass.h"
class Material;
class CollisionData;
//class DescriptorSet;

enum class TerrainType {
	CONTINGUOUS,
	PATTERN
};

#define TerrainUpdateNormals	1
#define TerrainUpdateVertices	2
#define TerrainUpdatePlanes		4
#define TerrainUpdateAll		7

#define TERRAIN_MAX_SIZE	65536
#define TERRAIN_LOG_CHUNK_SIZE	6
#define TERRAIN_CHUNK_SIZE	(1 << TERRAIN_LOG_CHUNK_SIZE)
#define TERRAIN_MAX_CHUNKS (TERRAIN_MAX_SIZE / TERRAIN_CHUNK_SIZE)


// a list of triangles for collision detection
class TriangleHull {
public:
	// large list of vertices
	vec3 *p;

	int num_vertices;
	int *index;

	int num_triangles;
	int *triangle_index;
	plane *pl;

	int num_edges;
	int *edge_index;
};

class Terrain : public Component {
public:
	Terrain();
	Terrain(int nx, int nz, const vec3& pattern, Material* material);
	Terrain(ResourceManager *resource_manager, const Path &filename);
	bool load(ResourceManager *resource_manager, const Path &filename, bool deep = true);
	~Terrain() override;
	void reset();
	void _cdecl update(int x1,int x2,int z1,int z2,int mode);
	float _cdecl gimme_height(const vec3 &p);
	float _cdecl gimme_height_n(const vec3 &p, vec3 &n);

	void get_triangle_hull(TriangleHull *hull, vec3 &pos, float radius);

	bool _cdecl trace(const vec3 &p1, const vec3 &p2, const vec3 &dir, float range, CollisionData &data, bool simple_test);

	void calc_detail(const vec3 &cam_pos);
	void prepare_draw(const vec3 &cam_pos);

	Path filename;
	TerrainType terrain_type;
	bool error;

	int num_x, num_z;
	Array<float> height;
	Array<vec3> vertex, normal;
	Array<plane> pl; // for collision detection
	owned<VertexBuffer> vertex_buffer;
	int chunk_lod[TERRAIN_MAX_CHUNKS][TERRAIN_MAX_CHUNKS];
	int chunk_lod_old[TERRAIN_MAX_CHUNKS][TERRAIN_MAX_CHUNKS];
	vec3 pattern, min, max;
	string vertex_shader_module;
	owned<Material> material;
	Path material_file;

	vec3 texture_scale[MATERIAL_MAX_TEXTURES];

	float dhx, dhz;


	bool changed;
	bool force_redraw;


	static const kaba::Class *_class;
};

struct XTerrainVBUpdater {
	struct Vertex {
		vec3 pos, n;
		float u, v;
	};
	Array<vec3> p,n;
	Array<float> uv;
	Array<Vertex> vertices;
	int mode = 0;
	int counter = 0;
	Terrain* terrain = nullptr;
	VertexBuffer* vb = nullptr;

	bool build_chunk(int chunk_no);
	void condense();
	void upload();

	int iterate(const vec3 &cam_pos);
};



