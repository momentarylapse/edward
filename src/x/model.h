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
#if !defined(MODEL_H__INCLUDED_)
#define MODEL_H__INCLUDED_


#include "../lib/base/base.h"
#include "../lib/file/path.h"
#include "../lib/math/math.h"

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
namespace nix {
	class VertexBuffer;
}
namespace Kaba {
	class Script;
}


#define MODEL_MAX_POLY_FACES			32
#define MODEL_MAX_POLY_EDGES			(MODEL_MAX_POLY_FACES*4)
#define MODEL_MAX_POLY_VERTICES_PER_FACE	16
#define MODEL_MAX_MOVE_OPS				8



class SubMesh {
public:
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
	Array<int> bone_index; // skeletal reference
	Array<vector> vertex;

	Array<SubMesh> sub;

	// bounding box
	vector min, max;

	Model *owner;

	Mesh* copy(Model *new_owner);
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
	int num_vertices;
	int *vertex;

	// non redundant edge list!
	int num_edges;
	int *edge_index;

	// "topology"
	bool *edge_on_face; // [edge * num_faces + face]
	int *faces_joining_edge; // [face1 * num_faces + face2]
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

// single animation
class Move {
public:
	int type; // skeletal/vertex
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
	vector *skel_dpos;
	quaternion *skel_ang;

	// vertex animation data
	struct {
		//Array<Array<vector>> dpos; // vertex animation data   [frame,vertex]
		vector* dpos;
	} mesh[4];
};

// types of animation
enum {
	MOVE_TYPE_NONE,
	MOVE_TYPE_VERTEX,
	MOVE_TYPE_SKELETAL
};

// commands for animation (move operations)
class MoveOperation {
public:
	int move, operation;
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
	vector pos;
	Model *model;
	// current skeletal data
	matrix dmatrix;
	quaternion cur_ang;
	vector cur_pos;
};

enum {
	MESH_HIGH,
	MESH_MEDIUM,
	MESH_LOW,
	MODEL_NUM_MESHES
};
#define MESH_DYNAMIC					8
#define MESH_DYNAMIC_VIEW_HIGH			(MESH_DYNAMIC | MESH_HIGH)
#define MESH_DYNAMIC_VIEW_MEDIUM		(MESH_DYNAMIC | MESH_MEDIUM)
#define MESH_DYNAMIC_VIEW_LOW			(MESH_DYNAMIC | MESH_LOW)
#define MESH_PHYSICAL					42
#define MESH_DYNAMIC_PHYSICAL			43

class ModelTemplate {
public:
	Path filename, script_filename;
	Array<TemplateDataScriptVariable> variables;
	Model *model;
	Array<ModelEffectData> fx;
	Array<Path> bone_model_filename;
	Array<Path> inventory_filename;
	Kaba::Script *script;

	ModelTemplate(Model *m);
};

class Model : public VirtualBase {
public:
	Model();
	~Model() override;

	void _cdecl __init__();
	void _cdecl __delete__() override;

	void load(const Path &filename);
	Model *copy(Model *pre_allocated = NULL);
	void reset_data();
	void _cdecl make_editable();
	//void Update();

	static bool AllowDeleteRecursive;

	// animate me
	void do_animation(float elapsed);

	// skeleton
	vector _cdecl _GetBonePos(int index);
	void _cdecl SetBoneModel(int index, Model *sub);

	// animation
	vector _cdecl GetVertex(int index,int mesh);

	// helper functions for collision detection
	void _UpdatePhysAbsolute_();
	void _ResetPhysAbsolute_();

	bool _cdecl Trace(const vector &p1, const vector &p2, const vector &dir, float range, TraceData &data, bool simple_test);
	bool _cdecl TraceMesh(const vector &p1, const vector &p2, const vector &dir, float range, TraceData &data, bool simple_test);

	// animation
	void _cdecl ResetAnimation();
	bool _cdecl IsAnimationDone(int operation_no);
	bool _cdecl Animate(int mode, float param1, float param2, int move_no, float &time, float elapsed, float vel_param, bool loop);
	int _cdecl GetFrames(int move_no);
	void _cdecl BeginEditAnimation();
	void _cdecl BeginEdit(int detail);
	void _cdecl EndEdit(int detail);

	// drawing
	void update_vertex_buffer(int mat_no, int detail);
	void draw(int detail, bool set_fx, bool allow_shadow);
	void draw_simple(int material, int detail);

	// visible skins (shared)
	Mesh *mesh[MODEL_NUM_MESHES];
	bool mesh_is_reference[MODEL_NUM_MESHES];

	// material (own)
	Array<Material*> material;

	// dynamical data (own)
	Array<vector> vertex_dyn[MODEL_NUM_MESHES]; // here the animated vertices are stored before rendering
	Array<vector> normal_dyn[MODEL_NUM_MESHES];
	
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
	int _detail_; // per view (more than once a frame...)

	// effects (own)
	Array<Fx::Effect*> fx;

	// skeleton (own)
	Array<Bone> bone;
	Array<vector> bone_pos_0;

	// move operations
	struct AnimationData {
		int num_operations;
		MoveOperation operation[MODEL_MAX_MOVE_OPS];
		MetaMove *meta;
	} anim;

	btRigidBody* body;
	btCollisionShape* colShape;

	virtual void _cdecl on_init(){}
	virtual void _cdecl on_delete(){}
	virtual void _cdecl on_collide_m(Model *o){}
	virtual void _cdecl on_collide_t(Terrain *t){}
	virtual void _cdecl on_iterate(float dt){}
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


// move operations
enum {
	MOVE_OP_SET,			// overwrite
	MOVE_OP_SET_NEW_KEYED,	// overwrite, if current doesn't equal 0
	MOVE_OP_SET_OLD_KEYED,	// overwrite, if last equals 0
	MOVE_OP_ADD_1_FACTOR,	// w = w_old         + w_new * f
	MOVE_OP_MIX_1_FACTOR,	// w = w_old * (1-f) + w_new * f
	MOVE_OP_MIX_2_FACTOR	// w = w_old * a     + w_new * b
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


#endif

