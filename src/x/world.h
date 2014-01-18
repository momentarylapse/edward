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
};
extern WorldData World;

extern int PhysicsNumSteps, PhysicsNumLinkSteps;

#ifdef _X_ALLOW_PHYSICS_DEBUG_
	extern int PhysicsTimer;
	extern float PhysicsTimeCol, PhysicsTimePhysics, PhysicsTimeLinks;
	extern sCollisionData PhysicsDebugColData;
	extern bool PhysicsStopOnCollision;
#endif


void GodInit();
void GodReset();
void GodResetLevelData();
bool GodLoadWorldFromLevelData();
bool GodLoadWorld(const string &filename);

extern bool GodNetMsgEnabled;
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
	TraceTypeNone = -1,
	TraceTypeTerrain,
	TraceTypeModel
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

	// content of the world
extern Array<Object*> Objects;
extern Object *Ego;
extern Object *terrain_object;

// esotherical (not in the world)
extern bool AddAllObjectsToLists;

// music fields
extern int NumMusicFields;
extern MusicField MusicFieldGlobal,MusicFields[GOD_MAX_MUSICFIELDS];
extern int MusicCurrent;

// force fields
extern int NumForceFields;
extern GodForceField *ForceField[GOD_MAX_FORCEFIELDS];
extern MusicField *MusicFieldCurrent;

extern GodLevelData LevelData;


extern Array<Terrain*> Terrains;

// network messages
struct GodNetMessage
{
	int msg, arg_i[4];
	string arg_s;
};
extern Array<GodNetMessage> GodNetMessages;

Object *_cdecl _CreateObject(const string &filename, const vector &pos);


#define FFKindRadialConst		0
#define FFKindRadialLinear		1
#define FFKindRadialQuad		2
#define FFKindDirectionalConst	10
#define FFKindDirectionalLinear	11
#define FFKindDirectionalQuad	12

#define NetMsgCreateObject		1000
#define NetMsgDeleteObject		1002
#define NetMsgSCText			2000

#define FieldKindLight		0
#define FieldKindMusic		1
#define FieldKindWeather	2
#define FieldKindURW		3

#endif
