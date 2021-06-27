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
#include "Material.h"
class Material;
class CollisionData;
namespace nix {
	class VertexBuffer;
	class UniformBuffer;
	//class DescriptorSet;
};

class btRigidBody;
class btCollisionShape;

enum {
	TerrainTypeContingous,
	TerrainTypePattern
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

class Terrain : public Entity {
public:
	Terrain();
	Terrain(const Path &filename, const vector &pos);
	bool load(const Path &filename, const vector &pos, bool deep = true);
	~Terrain();
	void reset();
	void _cdecl update(int x1,int x2,int z1,int z2,int mode);
	float _cdecl gimme_height(const vector &p);
	float _cdecl gimme_height_n(const vector &p, vector &n);

	void get_triangle_hull(TriangleHull *hull, vector &pos, float radius);

	bool _cdecl trace(const vector &p1, const vector &p2, const vector &dir, float range, CollisionData &data, bool simple_test);

	void calc_detail();
	void build_vertex_buffer();
	void draw();

	Path filename;
	int type;
	bool error;

	vector pos;
	int num_x, num_z;
	Array<float> height;
	Array<vector> vertex, normal;
	Array<plane> pl; // for collision detection
	nix::VertexBuffer *vertex_buffer;
	nix::UniformBuffer *ubo;
	//vulkan::DescriptorSet *dset;
	int partition[128][128], partition_old[128][128];
	vector pattern, min, max;
	Material *material;
	Path material_file;

	Path texture_file[MATERIAL_MAX_TEXTURES];
	vector texture_scale[MATERIAL_MAX_TEXTURES];

	float dhx, dhz;


	btRigidBody* body;
	btCollisionShape* colShape;


	bool changed;
	bool redraw, force_redraw;
	vector pos_old;
};



