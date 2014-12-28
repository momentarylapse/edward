/*----------------------------------------------------------------------------*\
| God                                                                          |
| -> manages objetcs and interactions                                          |
| -> loads and stores the world data (level)                                   |
|                                                                              |
| vital properties:                                                            |
|                                                                              |
| last updated: 2008.12.06 (c) by MichiSoft TM                                 |
\*----------------------------------------------------------------------------*/
#if !defined(GOD_H__INCLUDED_)
#define GOD_H__INCLUDED_

#define GOD_MAX_FORCEFIELDS		64
#define GOD_MAX_MUSIC_FILES		64
#define GOD_MAX_NET_MSGS		64
#define GOD_MAX_MUSICFIELDS		64


#include "../lib/base/base.h"
#include "../lib/math/math.h"

class Model;
class Object;
class Material;
class Terrain;
namespace Light{
	class Light;
};

class GodForceField
{
public:
	vector Pos,Dir;
	int Shape,Kind;
	float Radius,Vel,Acc,TimeToLife;
	bool Visible;
};

class Fog
{
public:
	bool enabled;
	int mode;
	float start, end, density;
	color _color;
};

class LevelDataTerrain
{
public:
	string filename;
	vector pos;
};

class LevelDataObject
{
public:
	string filename, name;
	vector pos, ang, vel, rot;
};

class LevelDataScriptRule
{
public:
	string function;
	int location;
};

class GodLevelData
{
public:
	string world_filename;
	Array<string> skybox_filename;
	Array<vector> skybox_ang;
	color background_color;
	Array<LevelDataObject> object;
	Array<LevelDataTerrain> terrain;
	int ego_index;
	Array<string> script_filename;
	Array<float> script_var;
	
	bool sun_enabled;
	color sun_color[3];
	vector sun_ang;
	color ambient;

	bool physics_enabled;
	vector gravity;
	Fog fog;
};

struct MusicField
{
	vector PosMin,PosMax;
	int NumMusicFiles;
	string MusicFile[16];
};

struct PartialModel{
	Model *model;
	Material *material;
	int mat_index;
	float d;
	bool shadow, transparent;
};

// network messages
struct GodNetMessage
{
	int msg, arg_i[4];
	string arg_s;
};



// game data
class WorldData
{
public:
	string filename;
	color background;
	Array<Model*> skybox;
	color ambient;
	Fog fog;
	Light::Light *sun;
	float speed_of_sound;

	vector gravity;
	Array<float> var;


	int physics_num_steps, physics_num_link_steps;

	#ifdef _X_ALLOW_PHYSICS_DEBUG_
		int PhysicsTimer;
		float PhysicsTimeCol, PhysicsTimePhysics, PhysicsTimeLinks;
		sCollisionData PhysicsDebugColData;
		bool PhysicsStopOnCollision;
	#endif

	bool net_msg_enabled;
	Array<GodNetMessage> net_messages;

	// content of the world
	Array<Object*> objects;
	Object *ego;
	Object *terrain_object;

	Array<Terrain*> terrains;


	// esotherical (not in the world)
	bool add_all_objects_to_lists;

	// music fields
	int NumMusicFields;
	MusicField MusicFieldGlobal,MusicFields[GOD_MAX_MUSICFIELDS];
	int MusicCurrent;

	// force fields
	int NumForceFields;
	GodForceField *ForceField[GOD_MAX_FORCEFIELDS];
	MusicField *MusicFieldCurrent;
};
extern WorldData World;


void GodInit();
void GodReset();
void GodResetLevelData();
bool GodLoadWorldFromLevelData();
bool GodLoadWorld(const string &filename);

Object* _cdecl GodCreateObject(const string &filename, const string &name, const vector &pos, const quaternion &ang, int w_index=-1);
void GodRegisterObject(Model *m, int index = -1);
void GodUnregisterObject(Model *m);
void GodRegisterModel(Model *m);
void GodUnregisterModel(Model *m);
void AddNewForceField(vector pos,vector dir,int kind,int shape,float r,float v,float a,bool visible,float t);
void _cdecl WorldShiftAll(const vector &dpos);
//void DoSounds();
void SetSoundState(bool paused,float scale,bool kill,bool restart);
vector _cdecl GetG(vector &pos);
void GodCalcMove();
void GodCalcMove2(); // debug
void GodDoCollisionDetection();
void GodDraw();
Object *_cdecl GetObjectByName(const string &name);
bool _cdecl NextObject(Object **o);
void _cdecl GodObjectEnsureExistence(int id);
int _cdecl GodFindObjects(vector &pos, float radius, int mode, Array<Object*> &a);

void Test4Ground(Object *o);
void Test4Object(Object *o1,Object *o2);


// what is hit (TraceData.type)
enum
{
	TRACE_TYPE_NONE = -1,
	TRACE_TYPE_TERRAIN,
	TRACE_TYPE_MODEL
};

class TraceData
{
public:
	int type;
	vector point;
	Terrain *terrain;
	Model *model;
	Model *object;
};
bool _cdecl GodTrace(const vector &p1, const vector &p2, TraceData &d, bool simple_test, Model *o_ignore = NULL);


extern GodLevelData LevelData;


Object *_cdecl _CreateObject(const string &filename, const vector &pos);


/*#define FFKindRadialConst		0
#define FFKindRadialLinear		1
#define FFKindRadialQuad		2
#define FFKindDirectionalConst	10
#define FFKindDirectionalLinear	11
#define FFKindDirectionalQuad	12*/

enum
{
	NET_MSG_CREATE_OBJECT = 1000,
	NET_MSG_DELETE_OBJECT = 1002,
	NET_MSG_SCTEXT = 2000
};

#endif
