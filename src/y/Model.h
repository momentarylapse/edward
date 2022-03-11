/*----------------------------------------------------------------------------*\
| Model                                                                        |
| -> can be a skeleton                                                         |
|    -> sub-models                                                             |
|    -> animation data                                                         |
| -> model                                                                     |
|    -> vertex and triangle data for rendering                                 |
|    -> consists of 4 skins                                                    |
|       -> 0-2 = visible detail levels (LOD) 0=high detail                     |
|       -> 3   = dynamical (for animation)                                     |
|    -> seperate physical skin (vertices, balls and convex polyeders)          |
|       -> absolute vertex positions in a seperate structure                   |
| -> strict seperation:                                                        |
|    -> dynamical data (changed during use)                                    |
|    -> unique data (only one instance for several copied models)              |
| -> can contain effects (fire, light, water,...)                              |
| MOSTLY WRONG!!!!                                                             |
|                                                                              |
| vital properties:                                                            |
|  - vertex buffers get filled temporaryly per frame                           |
|                                                                              |
| last update: 2008.01.22 (c) by MichiSoft TM                                  |
\*----------------------------------------------------------------------------*/
#pragma once


//#include "Entity3D.h"
#include "../graphics-fwd.h"
#include "../y/Component.h"
#include "../lib/base/base.h"
#include "../lib/base/pointer.h"
#include "../lib/file/path.h"
#include "../lib/math/matrix.h"
#include "../lib/math/matrix3.h"
#include "../lib/math/plane.h"
#include "../lib/math/vec4.h"
#include "../lib/image/color.h"


class Model;
class Material;
class TraceData;
class TemplateDataScriptVariable;
class ModelTemplate;
class MeshCollider;
class SolidBody;
class Animator;
namespace kaba {
	class Module;
}



class Mesh;


class SubMesh {
public:
	SubMesh();
	void create_vb(bool animated);
	void update_vb(Mesh *mesh, bool animated);

	int num_triangles;

	// vertices
	Array<int> triangle_index;

	// texture mapping
	Array<float> skin_vertex;

	// normals
	Array<vector> normal;

	VertexBuffer *vertex_buffer;

	// refill the vertex buffer etc...
	bool force_update;
};

// visual skin
class Mesh {
public:
	void create_vb(bool animated);
	void update_vb(bool animated);
	void post_process(bool animated);

	Array<ivec4> bone_index; // skeletal reference
	Array<vec4> bone_weight;
	Array<vector> vertex;

	Array<SubMesh> sub;

	// bounding box
	vector min, max;

	Model *owner;

	Mesh *copy(Model *new_owner);
};

enum {
	MESH_HIGH,
	MESH_MEDIUM,
	MESH_LOW,
	MODEL_NUM_MESHES,

	MESH_PHYSICAL = 42 // for edward
};

class Model : public Component {
public:
	Model();
	~Model() override;

	void _cdecl __init__();
	void _cdecl __delete__() override;

	Model *copy(Model *pre_allocated = NULL);
	void reset_data();
	void _cdecl make_editable();
	//void Update();
	void _cdecl begin_edit(int detail);
	void _cdecl end_edit(int detail);

	static bool AllowDeleteRecursive;

	// animation
	vector _cdecl get_vertex(int index);

	// helper functions for collision detection
	void _UpdatePhysAbsolute_();
	void _ResetPhysAbsolute_();

	bool _cdecl trace(const vector &p1, const vector &p2, const vector &dir, float range, TraceData &data, bool simple_test);
	bool _cdecl trace_mesh(const vector &p1, const vector &p2, const vector &dir, float range, TraceData &data, bool simple_test);

	// drawing
	//void update_vertex_buffer(int mat_no, int detail);

	// visible skins (shared)
	Mesh *mesh[MODEL_NUM_MESHES];

	// material (own)
	Array<Material*> material;

	// properties
	struct Properties {
		float detail_dist[MODEL_NUM_MESHES];
		float radius;
		vector min, max; // "bounding box"
		bool allow_shadow;
	} prop;

	bool is_copy;

	// script data (own)
	struct ScriptData {
		string name;
	} script_data;

	bool visible;

	matrix _matrix, matrix_old;
	void update_matrix();

	// template
	shared<ModelTemplate> _template;
	Path filename();

	// engine data
	bool registered;
	bool _detail_needed_[MODEL_NUM_MESHES]; // per frame


	static const kaba::Class *_class;
};


// types of shading/normal vectors
enum {
	NORMAL_MODE_SMOOTH,
	NORMAL_MODE_HARD,
	NORMAL_MODE_SMOOTH_EDGES,
	NORMAL_MODE_ANGULAR,
	NORMAL_MODE_PER_VERTEX,
	NORMAL_MODE_PRE = 16,
};


enum {
	FX_TYPE_SCRIPT,
	FX_TYPE_SOUND,
	FX_TYPE_LIGHT,
	FX_TYPE_FORCEFIELD,
	FX_TYPE_FOG
};

// observers for collision detection
void DoCollisionObservers();
extern int NumObservers;

#define SET_MATERIAL_ALL				0xffff
#define SET_MATERIAL_FRICTION			1
#define SET_MATERIAL_COLORS				2
#define SET_MATERIAL_TRANSPARENCY		4
#define SET_MATERIAL_APPEARANCE			6


