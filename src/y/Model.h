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


#include "../lib/base/base.h"
#include "../lib/file/path.h"
#include "../lib/math/vector.h"
#include "../lib/math/matrix.h"
#include "../lib/math/matrix3.h"
#include "../lib/math/plane.h"
#include "../lib/math/quaternion.h"
#include "../lib/image/color.h"
#include "../y/Entity.h"

class btRigidBody;
class btCollisionShape;


class Model;
namespace Fx {
	class Effect;
}
class Material;
class TraceData;
class Terrain;
class TemplateDataScriptVariable;
class CollisionData;
namespace nix {
	class VertexBuffer;
	class Buffer;
}
namespace kaba {
	class Script;
}


#define MODEL_MAX_POLY_FACES			32
#define MODEL_MAX_POLY_EDGES			(MODEL_MAX_POLY_FACES*4)
#define MODEL_MAX_POLY_VERTICES_PER_FACE	16
#define MODEL_MAX_MOVE_OPS				8

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

	nix::VertexBuffer *vertex_buffer;

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

// the face of a polyhedron (=> a polygon)
class ConvexPolyhedronFace {
public:
	int num_vertices;
	int index[MODEL_MAX_POLY_VERTICES_PER_FACE];
	plane pl; // in model space
};

// a convex polyhedron (for the physical skin)
class ConvexPolyhedron {
public:
	int num_faces;
	ConvexPolyhedronFace face[MODEL_MAX_POLY_FACES];

	// non redundant vertex list!
	Array<int> vertex;

	// non redundant edge list!
	int num_edges;
	Array<int> edge_index;

	// "topology"
	Array<bool> edge_on_face; // [edge * num_faces + face]
	Array<int> faces_joining_edge; // [face1 * num_faces + face2]
};

// a ball (for the physical skin)
class Ball {
public:
	int index;
	float radius;
};

// a cylinder (for the physical skin)
class Cylinder {
public:
	int index[2];
	float radius;
	bool round;
};

// data for collision detection
class PhysicalMesh {
public:
	Array<int> bone_nr;
	Array<vector> vertex; // original vertices
	Array<vector> vertex_dyn; // here the animated vertices are stored before collision detection

	/*int num_triangles;
	unsigned short *triangle_index;*/

	/*int NumEdges;
	unsigned short *EdgeIndex;*/

	Array<Ball> balls;

	Array<Cylinder> cylinders;

	Array<ConvexPolyhedron> poly;
};

// physical skin, but in world coordinates
class PhysicalMeshAbsolute {
public:
	bool is_ok;
	Array<vector> p;
	Array<plane> pl;
};

enum class AnimationType {
	NONE,
	VERTEX,
	SKELETAL
};

// single animation
class Move {
public:
	string name;
	int id;
	AnimationType type;
	int num_frames;
	int frame0;

	// properties
	float frames_per_sec_const, frames_per_sec_factor;
	bool inter_quad, inter_loop;
};

// a list of animations
class MetaMove {
public:
	MetaMove();
	// universal animation data
	Array<Move> move;

	int num_frames_skeleton, num_frames_vertex;


	// skeletal animation data
	//Array<Array<vector>> skel_dpos; //   [frame,bone]
	//Array<Array<quaternion>> skel_ang; //   [frame,bone]
	Array<vector> skel_dpos;
	Array<quaternion> skel_ang;

	// vertex animation data
	struct {
		//Array<Array<vector>> dpos; // vertex animation data   [frame,vertex]
		Array<vector> dpos;
	} mesh[4];
};

// commands for animation (move operations)
class MoveOperation {
public:
	// move operations
	enum class Command {
		SET,			// overwrite
		SET_NEW_KEYED,	// overwrite, if current doesn't equal 0
		SET_OLD_KEYED,	// overwrite, if last equals 0
		ADD_1_FACTOR,	// w = w_old         + w_new * f
		MIX_1_FACTOR,	// w = w_old * (1-f) + w_new * f
		MIX_2_FACTOR	// w = w_old * a     + w_new * b
	};
	int move;
	Command command;
	float time, param1, param2;
};

// to store data to create effects (when copying models)
class ModelEffectData {
public:
	int vertex;
	int type;
	Path filename;
	float radius, speed;
	color am, di, sp;
};

class Bone {
public:
	int parent;
	vector delta_pos;
	vector rest_pos;
	Model *model;
	// current skeletal data
	quaternion cur_ang;
	vector cur_pos;
	matrix dmatrix;
};

enum {
	MESH_HIGH,
	MESH_MEDIUM,
	MESH_LOW,
	MODEL_NUM_MESHES,

	MESH_PHYSICAL = 42 // for edward
};

class ModelTemplate {
public:
	Path filename, script_filename;
	Array<TemplateDataScriptVariable> variables;
	Model *model;
	Array<ModelEffectData> fx;
	Array<Path> bone_model_filename;
	Array<Path> inventory_filename;
	kaba::Script *script;

	ModelTemplate(Model *m);
};

class Model : public Entity {
public:
	Model();
	~Model() override;

	void _cdecl __init__();
	void _cdecl __delete__() override;

	Model *copy(Model *pre_allocated = NULL);
	void reset_data();
	void _cdecl make_editable();
	//void Update();

	static bool AllowDeleteRecursive;

	// animation
	vector _cdecl get_vertex(int index);

	// skeleton
	vector _cdecl get_bone_rest_pos(int index) const;
	void _cdecl set_bone_model(int index, Model *sub);

	// helper functions for collision detection
	void _UpdatePhysAbsolute_();
	void _ResetPhysAbsolute_();

	bool _cdecl trace(const vector &p1, const vector &p2, const vector &dir, float range, TraceData &data, bool simple_test);
	bool _cdecl trace_mesh(const vector &p1, const vector &p2, const vector &dir, float range, TraceData &data, bool simple_test);

	// animation
	void _cdecl reset_animation();
	bool _cdecl is_animation_done(int operation_no);
	bool _cdecl animate_x(MoveOperation::Command cmd, float param1, float param2, int move_no, float &time, float dt, float vel_param, bool loop);
	bool _cdecl animate(MoveOperation::Command cmd, int move_no, float &time, float dt, bool loop);
	int _cdecl get_frames(int move_no);
	void _cdecl begin_edit_animation();
	void _cdecl begin_edit(int detail);
	void _cdecl end_edit(int detail);
	void do_animation(float elapsed);

	// drawing
	//void update_vertex_buffer(int mat_no, int detail);

	// visible skins (shared)
	Mesh *mesh[MODEL_NUM_MESHES];

	// material (own)
	Array<Material*> material;
	
	// physical skin (shared)
	PhysicalMesh *phys;
	bool phys_is_reference;
	PhysicalMeshAbsolute phys_absolute;

	// properties
	struct Properties {
		float detail_dist[MODEL_NUM_MESHES];
		float radius;
		vector min, max; // "bounding box"
		bool allow_shadow;
		bool flexible;
	} prop;

	bool is_copy;

	// physics
	struct PhysicsData {
		float mass, mass_inv, g_factor;
		matrix3 theta_0, theta, theta_inv;
		bool active, passive;
		bool test_collisions;
	} physics_data;

	// script data (own)
	struct ScriptData {
		string name, description;
		Array<Model*> inventary;
		Array<float> var;
	} script_data;

	int object_id;
	Model *parent;
	Model *_cdecl root();
	bool on_ground, visible, rotating, moved, frozen;
	float time_till_freeze;
	int ground_id;
	vector ground_normal;


	vector pos, vel, vel_surf, /*pos_old,*/ acc;
	quaternion ang /*,ang_old*/;
	vector rot;
	matrix _matrix, matrix_old;

	vector force_int, torque_int;
	vector force_ext, torque_ext;

	// template (shared)
	ModelTemplate *_template;
	Path _cdecl filename();

	// engine data
	bool registered;
	bool _detail_needed_[MODEL_NUM_MESHES]; // per frame

	// effects (own)
	Array<Fx::Effect*> fx;

	// skeleton (own)
	Array<Bone> bone;

	// move operations
	struct AnimationData {
		int num_operations;
		MoveOperation operation[MODEL_MAX_MOVE_OPS];
		MetaMove *meta; // shared

		// dynamical data (own)
		//Mesh *mesh[MODEL_NUM_MESHES]; // here the animated vertices are stored before rendering

		Array<matrix> dmatrix;
		nix::Buffer *buf;
	} anim;
	bool uses_bone_animations() const;

	btRigidBody* body;
	btCollisionShape* colShape;

	virtual void _cdecl on_init() {}
	virtual void _cdecl on_delete() {}
	virtual void _cdecl on_collide(const CollisionData &col) {}
	virtual void _cdecl on_iterate(float dt) {}
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


