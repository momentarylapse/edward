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
#include "../lib/math/math.h"

class Model;
namespace Fx{
class Effect;
};
class Material;
class TraceData;
class Terrain;
namespace nix{
	class VertexBuffer;
};


#define MODEL_MAX_POLY_FACES			32
#define MODEL_MAX_POLY_EDGES			(MODEL_MAX_POLY_FACES*4)
#define MODEL_MAX_POLY_VERTICES_PER_FACE	16
#define MODEL_MAX_MOVE_OPS				8



template <class T>
class CopyAsRefArray : public Array<T>
{
	public:
		void operator = (const CopyAsRefArray<T> &a)
		{
			this->set_ref(a);
		}
		void forget()
		{
			Array<T>::data = NULL;
			Array<T>::allocated = 0;
			Array<T>::num = 0;
		}
		void make_own()
		{
			T *dd = (T*)Array<T>::data;
			int n = Array<T>::num;
			forget();
			Array<T>::resize(n);
			for (int i=0;i<Array<T>::num;i++)
				(*this)[i] = dd[i];
		}
};


class SubSkin
{
public:
	int num_triangles;
	
	// vertices
	CopyAsRefArray<int> triangle_index;
	
	// texture mapping
	CopyAsRefArray<float> skin_vertex;

	// normals
	CopyAsRefArray<vector> normal;

	nix::VertexBuffer *vertex_buffer;

	// refill the vertex buffer etc...
	bool force_update;
};

// visual skin
class Skin
{
public:
	CopyAsRefArray<int> bone_index; // skeletal reference
	CopyAsRefArray<vector> vertex;

	CopyAsRefArray<SubSkin> sub;
	
	// bounding box
	vector min, max;

	bool copy_as_ref;
};

// the face of a polyhedron (=> a polygon)
class ConvexPolyhedronFace
{
public:
	int num_vertices;
	int index[MODEL_MAX_POLY_VERTICES_PER_FACE];
	plane pl; // in model space
};

// a convex polyhedron (for the physical skin)
class ConvexPolyhedron
{
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
class Ball
{
public:
	int index;
	float radius;
};

// data for collision detection
class PhysicalSkin
{
public:
	int num_vertices;
	int *bone_nr;
	vector *vertex; // original vertices
	vector *vertex_dyn; // here the animated vertices are stored before collision detection

	/*int num_triangles;
	unsigned short *triangle_index;*/

	/*int NumEdges;
	unsigned short *EdgeIndex;*/

	int num_balls;
	Ball *ball;

	int num_polys;
	ConvexPolyhedron *poly;
};

// physical skin, but in world coordinates
class PhysicalSkinAbsolute
{
public:
	bool is_ok;
	vector *p;
	plane *pl;
};

// single animation
class Move
{
public:
	int type; // skeletal/vertex
	int num_frames;
	int frame0;

	// properties
	float frames_per_sec_const, frames_per_sec_factor;
	bool inter_quad, inter_loop;
};

// a list of animations
class MetaMove
{
public:
	// universal animation data
	int num_moves;
	Move *move;

	int num_frames_skeleton, num_frames_vertex;

	// skeletal animation data
	vector *skel_dpos; //   [frame * num_bones + bone]
	quaternion *skel_ang; //   [frame * num_bones + bone]

	// vertex animation data
	struct{
		vector *dpos; // vertex animation data   [frame * num_vertices + vertex]
	}skin[4];
	void reset()
	{	memset(this, 0, sizeof(MetaMove));	}
};

// types of animation
enum
{
	MOVE_TYPE_NONE,
	MOVE_TYPE_VERTEX,
	MOVE_TYPE_SKELETAL
};

// commands for animation (move operations)
class MoveOperation
{
public:
	int move, operation;
	float time, param1, param2;
};

// to store data to create effects (when copying models)
class ModelEffectData
{
public:
	int vertex;
	int type;
	string filename;
	float radius, speed;
	color am, di, sp;
};

class Bone
{
public:
	int parent;
	vector pos;
	Model *model;
	// current skeletal data
	matrix dmatrix;
	quaternion cur_ang;
	vector cur_pos;
};

enum{
	SKIN_HIGH,
	SKIN_MEDIUM,
	SKIN_LOW,
	MODEL_NUM_SKINS
};
#define SKIN_DYNAMIC					8
#define SKIN_DYNAMIC_VIEW_HIGH			(SKIN_DYNAMIC | SKIN_HIGH)
#define SKIN_DYNAMIC_VIEW_MEDIUM		(SKIN_DYNAMIC | SKIN_MEDIUM)
#define SKIN_DYNAMIC_VIEW_LOW			(SKIN_DYNAMIC | SKIN_LOW)
#define SKIN_PHYSICAL					42
#define SKIN_DYNAMIC_PHYSICAL			43

class ModelTemplate
{
public:
	string filename, script_filename;
	Model *model;
	Array<ModelEffectData> fx;
	void *script;

	ModelTemplate(Model *m)
	{
		filename = "";
		script_filename = "";
		fx.clear();
		script = NULL;
		model = m;
	}
};

class Model : public VirtualBase
{
public:
	Model(const string &filename);
	Model();
	void Load(const string &filename);
	Model *GetCopy(bool allow_script_init);
	void ResetData();
	void _cdecl MakeEditable();
	void SetMaterial(Material *material, int mode);
	//void Update();
	void reset();
	virtual ~Model();
	void DeleteBaseModel();

	static bool AllowDeleteRecursive;

	void _cdecl __init__();
	virtual void _cdecl __delete__();

	// animate me
	void CalcMove(float elapsed);

	// skeleton
	vector _cdecl _GetBonePos(int index);
	void _cdecl SetBoneModel(int index, Model *sub);

	// animation
	vector _cdecl GetVertex(int index,int skin);

	// helper functions for collision detection
	void _UpdatePhysAbsolute_();
	void _ResetPhysAbsolute_();

	bool _cdecl Trace(const vector &p1, const vector &p2, const vector &dir, float range, TraceData &data, bool simple_test);

	// animation
	void _cdecl ResetAnimation();
	bool _cdecl IsAnimationDone(int operation_no);
	bool _cdecl Animate(int mode, float param1, float param2, int move_no, float &time, float elapsed, float vel_param, bool loop);
	int _cdecl GetFrames(int move_no);
	void _cdecl BeginEditAnimation();
	void _cdecl BeginEdit(int detail);
	void _cdecl EndEdit(int detail);

	// drawing
	void Draw(int detail, bool set_fx, bool allow_shadow);
	void JustDraw(int material, int detail);

	// visible skins (shared)
	Skin *skin[MODEL_NUM_SKINS];
	bool skin_is_reference[MODEL_NUM_SKINS];
	// material (shared)
	CopyAsRefArray<Material> material;
	bool material_is_reference;
	// dynamical data (own)
	vector *vertex_dyn[MODEL_NUM_SKINS]; // here the animated vertices are stored before rendering
	vector *normal_dyn[MODEL_NUM_SKINS];
	
	// physical skin (shared)
	PhysicalSkin *phys;
	bool phys_is_reference;
	PhysicalSkinAbsolute phys_absolute;

	// properties
	float detail_dist[MODEL_NUM_SKINS];
	float radius;
	vector min, max; // "bounding box"
	matrix3 theta_0, theta, theta_inv;
	bool test_collisions;
	bool allow_shadow;
	bool flexible;
	bool is_copy, error;

	// physics
	float mass, mass_inv, g_factor;
	bool active_physics, passive_physics;

	// script data (own)
	string name, description;
	Array<Model*> inventary;
	Array<float> script_var;

	int object_id;
	Model *parent;
	Model *_cdecl GetRoot();
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

	float rc_jump, rc_static, rc_sliding, rc_rolling;

	// template (shared)
	ModelTemplate *_template;
	string _cdecl GetFilename();

	// engine data
	bool registered;
	bool _detail_needed_[MODEL_NUM_SKINS]; // per frame
	int _detail_; // per view (more than once a frame...)

	// effects (own)
	Array<Fx::Effect*> fx;

	// skeleton (own)
	Array<Bone> bone;
	vector *bone_pos_0;

	// move operations
	int num_move_operations;
	MoveOperation move_operation[MODEL_MAX_MOVE_OPS];
	MetaMove *meta_move;

	/*dBodyID*/ void* body_id;
	/*dGeomID*/ void* geom_id;

	virtual void _cdecl OnInit(){}
	virtual void _cdecl OnDelete(){}
	virtual void _cdecl OnCollideM(Model *o){}
	virtual void _cdecl OnCollideT(Terrain *t){}
	virtual void _cdecl OnIterate(float dt){}
};


// types of shading/normal vectors
enum
{
	NORMAL_MODE_SMOOTH,
	NORMAL_MODE_HARD,
	NORMAL_MODE_SMOOTH_EDGES,
	NORMAL_MODE_ANGULAR,
	NORMAL_MODE_PER_VERTEX,
	NORMAL_MODE_PRE = 16,
};


// move operations
enum
{
	MOVE_OP_SET,			// overwrite
	MOVE_OP_SET_NEW_KEYED,	// overwrite, if current doesn't equal 0
	MOVE_OP_SET_OLD_KEYED,	// overwrite, if last equals 0
	MOVE_OP_ADD_1_FACTOR,	// w = w_old         + w_new * f
	MOVE_OP_MIX_1_FACTOR,	// w = w_old * (1-f) + w_new * f
	MOVE_OP_MIX_2_FACTOR	// w = w_old * a     + w_new * b
};

enum{
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

