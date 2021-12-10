/*----------------------------------------------------------------------------*\
| Terrain                                                                      |
| -> terrain of a world                                                        |
|                                                                              |
| vital properties:                                                            |
|                                                                              |
| last updated: 2008.06.23 (c) by MichiSoft TM                                 |
\*----------------------------------------------------------------------------*/
#pragma once

#include "../lib/base/base.h"
#include "../lib/file/path.h"
#include "../lib/math/vector.h"
#include "../y/Entity.h"
#include "../y/Component.h"
#include "Material.h"
#include "../graphics-fwd.h"
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


// a list of triangles for collision detection
class TriangleHull {
public:
	// large list of vertices
	vector *p;

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
	Terrain(const Path &filename);
	bool load(const Path &filename, bool deep = true);
	~Terrain();
	void reset();
	void _cdecl update(int x1,int x2,int z1,int z2,int mode);
	float _cdecl gimme_height(const vector &p);
	float _cdecl gimme_height_n(const vector &p, vector &n);

	void get_triangle_hull(TriangleHull *hull, vector &pos, float radius);

	bool _cdecl trace(const vector &p1, const vector &p2, const vector &dir, float range, CollisionData &data, bool simple_test);

	void calc_detail(const vector &cam_pos);
	void build_vertex_buffer();
	void prepare_draw(const vector &cam_pos);

	Path filename;
	TerrainType terrain_type;
	bool error;

	int num_x, num_z;
	Array<float> height;
	Array<vector> vertex, normal;
	Array<plane> pl; // for collision detection
	VertexBuffer *vertex_buffer;
	int partition[128][128], partition_old[128][128];
	vector pattern, min, max;
	Material *material;
	Path material_file;

	Path texture_file[MATERIAL_MAX_TEXTURES];
	vector texture_scale[MATERIAL_MAX_TEXTURES];

	float dhx, dhz;


	bool changed;
	bool redraw, force_redraw;
	vector pos_old;


	static const kaba::Class *_class;
};



