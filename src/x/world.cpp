/*----------------------------------------------------------------------------*\
| God                                                                          |
| -> manages objetcs and interactions                                          |
| -> loads and stores the world data (level)                                   |
|                                                                              |
| vital properties:                                                            |
|                                                                              |
| last updated: 2009.11.22 (c) by MichiSoft TM                                 |
\*----------------------------------------------------------------------------*/
#include <algorithm>
#include "../lib/file/file.h"
#include "world.h"
#include "object.h"
#include "model.h"
#include "model_manager.h"
#include "terrain.h"
#include "../meta.h"
#include "../lib/nix/nix.h"
#ifdef _X_ALLOW_X_
#include "../physics/physics.h"
#include "../physics/links.h"
#include "../physics/collision.h"
#include "../fx/fx.h"
#include "../fx/light.h"
#endif
#include "camera.h"




//#define _debug_matrices_


#ifdef USE_ODE
	#define dSINGLE
	#include <ode/ode.h>
	bool ode_world_created = false;
	dWorldID world_id;
	dSpaceID space_id;
	dJointGroupID contactgroup;
inline void qx2ode(quaternion *qq, dQuaternion q)
{
	q[0] = qq->w;
	q[1] = qq->x;
	q[2] = qq->y;
	q[3] = qq->z;
}
inline void qode2x(const dQuaternion q, quaternion *qq)
{
	qq->w = q[0];
	qq->x = q[1];
	qq->y = q[2];
	qq->z = q[3];
}
#endif

int PhysicsNumSteps, PhysicsNumLinkSteps;

#ifdef _X_ALLOW_PHYSICS_DEBUG_
	int PhysicsTimer;
	float PhysicsTimeCol, PhysicsTimePhysics, PhysicsTimeLinks;
	bool PhysicsStopOnCollision = false;
#endif


GodLevelData LevelData;
static int GodNumReservedObjects;


// game data
WorldData World;


#ifdef _X_ALLOW_X_
void DrawSplashScreen(const string &str, float per);
void ScriptingObjectInit(Object *o);
#include "../networking.h"
#else
void DrawSplashScreen(const string &str, float per){}
void ScriptingObjectInit(Object *o){}
#endif

// partial models
static Array<PartialModel> SortedNonTrans, SortedTrans;

// network messages
void AddNetMsg(int msg, int argi0, const string &args)
{
#ifdef _X_ALLOW_X_
	if ((!World.net_msg_enabled) || (!Net.Enabled))
		return;
	if (World.net_messages.num >= GOD_MAX_NET_MSGS){
		//for (int i=0;i<NetMsg.num_msgs;i++)
		//	...
		return;
	}
	GodNetMessage m;
	m.msg = msg;
	m.arg_i[0] = argi0;
	m.arg_s = args;
	World.net_messages.add(m);
#endif
}


int num_insane=0;

inline bool TestVectorSanity(vector &v, const char *name)
{
	if (inf_v(v)){
		num_insane++;
		v=v_0;
		if (num_insane>100)
			return false;
		msg_error(format("Vektor %s unendlich!!!!!!!",name));
		return true;
	}
	return false;
}

void TestObjectSanity(const char *str)
{
#ifdef _X_ALLOW_PHYSICS_DEBUG_
	for (int i=0;i<Objects.num;i++)
		if (Objects[i]){
			Object *o=Objects[i];
			/*if (((int)o>1500000000)||((int)o<10000000)){
				msg_write(str);
				msg_error("Objekt-Pointer kaputt!");
				msg_write(i);
				msg_write((int)o);
			}*/
			bool e=false;
			e|=TestVectorSanity(o->Pos,"Pos");
			e|=TestVectorSanity(o->Vel,"Vel");
			e|=TestVectorSanity(o->ForceExt,"ForceExt");
			e|=TestVectorSanity(o->TorqueExt,"TorqueExt");
			e|=TestVectorSanity(o->ForceInt,"ForceInt");
			e|=TestVectorSanity(o->TorqueInt,"TorqueInt");
			e|=TestVectorSanity(o->Ang,"Ang");
			e|=TestVectorSanity(o->Rot,"Rot");
			if (e){
				msg_write(string2("%s:  objekt[%d] (%s) unendlich!!!!",str,i,o->Name));
				HuiRaiseError("Physik...");
			}
		}
#endif
}







void GodInit()
{
	msg_db_f("GodInit",1);
	World.gravity = v_0;
	World.NumForceFields = 0;

	World.terrain_object = new Object();
	World.terrain_object->UpdateMatrix();

#if 0
	COctree *octree = new COctree(v_0, 100);
	sOctreeLocationData dummy_loc;
	vector min =vector(23,31,9);
	vector max =vector(40,50,39);
	octree->Insert(min, max, (void*)1, &dummy_loc);
	min =vector(23,31,9);
	max =vector(24,32,10);
	octree->Insert(min, max, (void*)2, &dummy_loc);

	Array<void*> a;
	vector pos = vector(24, 30, 20);
	octree->GetPointNeighbourhood(pos, 100, a);

	msg_write("---Octree-Test---");
	msg_write(a.num);
	for (int i=0;i<a.num;i++)
		msg_write(p2s(a[i]));
	//exit(0);
	msg_write("-----------------");
#endif

	
	

#ifdef USE_ODE
	contactgroup = dJointGroupCreate(0);
#endif
}

void GodReset()
{
	msg_db_f("GodReset",1);
	World.net_msg_enabled = false;
	World.net_messages.clear();

	// terrains
	msg_db_m("-terrains",2);
	for (int i=0;i<World.terrains.num;i++)
		delete(World.terrains[i]);
	World.terrains.clear();

	// objects
	msg_db_m("-objects",2);
	for (int i=0;i<World.objects.num;i++)
		if (World.objects[i])
			GodUnregisterObject(World.objects[i]); // actual deleting done by ModelManager
	World.objects.clear();
	GodNumReservedObjects = 0;
	
	SortedTrans.clear();
	SortedNonTrans.clear();


	// force fields
	msg_db_m("-force fields",2);
	for (int i=0;i<World.NumForceFields;i++)
		delete(World.ForceField[i]);
	World.NumForceFields = 0;
	
	msg_db_m("-stuff",2);

	// music
	/*if (meta->MusicEnabled){
		NixSoundStop(MusicCurrent);
	}*/

	// skybox
	//   (models deleted by meta)
	World.skybox.clear();
	

	// initial data for empty world...
	World.ambient = color(1,0.4f,0.4f,0.4f);
	World.fog._color = White;
	World.fog.mode = FogExp;
	World.fog.density = 0.001f;
	World.fog.enabled = false;
	World.fog.start = 0;
	World.fog.end = 100000;
	World.speed_of_sound = 1000;
	
	Engine.PhysicsEnabled = false;
	Engine.CollisionsEnabled = true;
	PhysicsNumSteps = 10;
	PhysicsNumLinkSteps = 5;


	// physics
	msg_db_m("-physics",2);
#ifdef _X_ALLOW_X_
	LinksReset();
#endif
#ifdef USE_ODE
	if (ode_world_created){
		dWorldDestroy(world_id);
		dSpaceDestroy(space_id);
	}
	world_id = dWorldCreate();
	space_id = dSimpleSpaceCreate(0);
	//space_id = dHashSpaceCreate(0);
	/*int m1, m2;
	dHashSpaceGetLevels(space_id, &m1, &m2);
	printf("hash:    %d  %d\n", m1, m2);*/
	ode_world_created = true;
	World.terrain_object->body_id = 0;
	msg_db_r("-dCreateSphere",2);
	World.terrain_object->geom_id = dCreateSphere(0, 1); // space, radius
	msg_db_l(2)
	dGeomSetBody((dGeomID)World.terrain_object->geom_id, (dBodyID)World.terrain_object->body_id);
#endif
}

void GodResetLevelData()
{
	LevelData.world_filename = "";
	LevelData.terrain.clear();
	LevelData.object.clear();
	LevelData.skybox_filename.clear();
	LevelData.skybox_ang.clear();
	LevelData.script_filename.clear();
	LevelData.script_var.clear();

	LevelData.ego_index = -1;
	LevelData.background_color = Gray;
	LevelData.sun_enabled = false;
	//LevelData.sun_color[3];
	LevelData.sun_ang = vector(1, 0, 0);
	LevelData.ambient = color(1, 0.4f, 0.4f, 0.4f);

	LevelData.gravity = v_0;
	LevelData.fog.enabled = false;
}

color ReadColor3(CFile *f)
{
	int c[3];
	for (int i=0;i<3;i++)
		c[i] = f->ReadFloat();
	return ColorFromIntRGB(c);
}

color ReadColor4(CFile *f)
{
	int c[4];
	for (int i=0;i<4;i++)
		c[i] = f->ReadFloat();
	return ColorFromIntARGB(c);
}

bool GodLoadWorldFromLevelData()
{
	msg_db_f("GodLoadWorldFromLevelData", 1);
	World.net_msg_enabled = false;
	bool ok = true;

	Engine.PhysicsEnabled = LevelData.physics_enabled;
	Engine.CollisionsEnabled = true;//LevelData.physics_enabled;
	World.gravity = LevelData.gravity;
	World.fog = LevelData.fog;

	// script var
	World.var = LevelData.script_var;

	// set up light (sun and ambient)
	World.ambient = LevelData.ambient;
	NixSetAmbientLight(World.ambient);

#ifdef _X_ALLOW_X_
	World.sun = new Light::Light;
	World.sun->SetColors(LevelData.sun_color[0],
	                     LevelData.sun_color[1],
	                     LevelData.sun_color[2]);
	World.sun->SetDirectional(- LevelData.sun_ang.ang2dir());
	World.sun->enabled = LevelData.sun_enabled;
#endif

	// skybox
	World.skybox.resize(LevelData.skybox_filename.num);
	for (int i=0;i<World.skybox.num;i++){
		World.skybox[i] = LoadModel(LevelData.skybox_filename[i]);
		if (World.skybox[i])
			QuaternionRotationV(World.skybox[i]->ang, LevelData.skybox_ang[i]);
		LevelData.skybox_filename[i].clear();
	}
	LevelData.skybox_filename.clear();
	LevelData.skybox_ang.clear();
	World.background = LevelData.background_color;

	// objects
	World.ego = NULL;
	World.objects.clear(); // make sure the "missing" objects are NULL
	World.objects.resize(LevelData.object.num);
	GodNumReservedObjects = LevelData.object.num;
	foreachi(LevelDataObject &o, LevelData.object, i)
		if (o.filename.num > 0){
			quaternion q;
			QuaternionRotationV(q, o.ang);
			Object *oo = GodCreateObject(o.filename, o.name, o.pos, q, i);
			ok &= (oo >= 0);
			if (oo){
				oo->vel = o.vel;
				oo->rot = o.rot;
			}
			if (LevelData.ego_index == i)
				World.ego = oo;
			if (i % 5 == 0)
				DrawSplashScreen("Objects", (float)i / (float)LevelData.object.num / 5 * 3);
		}
	LevelData.object.clear();
	World.add_all_objects_to_lists = true;

	// terrains
	foreachi(LevelDataTerrain &t, LevelData.terrain, i){
		DrawSplashScreen("Terrain...", 0.6f + (float)i / (float)LevelData.terrain.num * 0.4f);
		Terrain *tt = new Terrain(t.filename, t.pos);
		World.terrains.add(tt);
		ok &= !tt->error;
	}
	LevelData.terrain.clear();

	World.net_msg_enabled = true;
	return ok;
}

bool GodLoadWorld(const string &filename)
{
	msg_db_f("GodLoadWorld", 1);
	LevelData.world_filename = filename;

// read world file
//   and put the data into LevelData
	CFile *f = FileOpen(MapDir + filename + ".world");
	if (!f)
		return false;

	int ffv = f->ReadFileFormatVersion();
	if (ffv != 10){
		msg_error(format("wrong file format: %d (10 expected)", ffv));
		FileClose(f);
		return false;
	}
	bool ok = true;
	GodResetLevelData();

	// Terrains
	int n = f->ReadIntC();
	for (int i=0;i<n;i++){
		LevelDataTerrain t;
		t.filename = f->ReadStr();
		f->ReadVector(&t.pos);
		LevelData.terrain.add(t);
	}

	// Gravity
	f->ReadComment();
	f->ReadVector(&LevelData.gravity);

	// EgoIndex
	LevelData.ego_index = f->ReadIntC();

	// Background
	f->ReadComment();
	LevelData.background_color = ReadColor4(f);
	n = f->ReadInt();
	for (int i=0;i<n;i++){
		LevelData.skybox_filename.add(f->ReadStr());
		LevelData.skybox_ang.add(v_0);
	}

	// Fog
	LevelData.fog.enabled = f->ReadBoolC();
	LevelData.fog.mode = f->ReadWord();
	LevelData.fog.start = f->ReadFloat();
	LevelData.fog.end = f->ReadFloat();
	LevelData.fog.density = f->ReadFloat();
	LevelData.fog._color = ReadColor4(f);

	// Music
	World.MusicFieldGlobal.NumMusicFiles = f->ReadIntC();
	for (int i=0;i<World.MusicFieldGlobal.NumMusicFiles;i++)
		World.MusicFieldGlobal.MusicFile[i] = f->ReadStr();
	World.MusicFieldCurrent = &World.MusicFieldGlobal;
	World.MusicCurrent = -1;

	// Objects
	n = f->ReadIntC();
	for (int i=0;i<n;i++){
		LevelDataObject o;
		o.filename = f->ReadStr();
		o.name = f->ReadStr();
		f->ReadVector(&o.pos);
		f->ReadVector(&o.ang);
		o.vel = v_0;
		o.rot = v_0;
		LevelData.object.add(o);
	}

	// Scripts
	n = f->ReadIntC();
	for (int i=0;i<n;i++){
		LevelData.script_filename.add(f->ReadStr());
		int nr = f->ReadInt();
		for (int j=0;j<nr;j++){
			f->ReadStr();
			f->ReadInt();
		}
	}

	// ScriptVars
	n = f->ReadIntC();
	LevelData.script_var.resize(n);
	for (int i=0;i<n;i++)
		LevelData.script_var[i] = f->ReadFloat();

	// light
	if (f->ReadStr() != "#"){
		// Sun
		LevelData.sun_enabled = f->ReadBool();
		for (int i=0;i<3;i++)
			LevelData.sun_color[i] = ReadColor3(f);
		LevelData.sun_ang.x = f->ReadFloat();
		LevelData.sun_ang.y = f->ReadFloat();
		LevelData.sun_ang.z = 0;
		// Ambient
		f->ReadComment();
		LevelData.ambient = ReadColor3(f);
		if (f->ReadStr() != "#"){
			LevelData.physics_enabled = f->ReadBool();
		}
	}

	// Fields
	/*NumMusicFields=0;
	int NumFields=f->ReadIntC();
	for (int i=0;i<NumFields;i++){
		vector min,max;
		int kind=f->ReadInt();
		min.x=(float)f->ReadInt();
		min.y=(float)f->ReadInt();
		min.z=(float)f->ReadInt();
		max.x=(float)f->ReadInt();
		max.y=(float)f->ReadInt();
		max.z=(float)f->ReadInt();
		if (kind==FieldKindLight){
			bool sunenabled;
			color ambient;
			sunenabled=f->ReadBool();
			float r=(float)f->ReadInt()/255.0f;
			float g=(float)f->ReadInt()/255.0f;
			float b=(float)f->ReadInt()/255.0f;
			ambient=color(0,r,g,b);
			fx->AddLighField(min,max,sunenabled,ambient);
		}
		if (kind==FieldKindMusic){
			MusicField[NumMusicFields].PosMin=min;
			MusicField[NumMusicFields].PosMax=max;
			MusicField[NumMusicFields].NumMusicFiles=f->ReadInt();
			for (int n=0;n<MusicField[NumMusicFields].NumMusicFiles;n++)
				strcpy(MusicField[NumMusicFields].MusicFile[n],f->ReadStr());
			NumMusicFields++;
		}
	}*/
	// Politics
	// Groups
	// Waypoints

	FileClose(f);
	World.MusicFieldGlobal.NumMusicFiles = 0;

	ok &= GodLoadWorldFromLevelData();

	return ok;
}

Object *GetObjectByName(const string &name)
{
	for (int i=0;i<World.objects.num;i++)
		if (World.objects[i])
			if (name == World.objects[i]->name){
				//msg_write(i);
				return World.objects[i];
			}
	msg_error(format("object \"%s\" not found!", name.c_str()));
	return NULL;
}

bool NextObject(Object **o)
{
	int id=-1;
	if (*o)
		id = (*o)->object_id;
	for (int i=id+1;i<World.objects.num;i++)
		if (World.objects[i]){
			*o = World.objects[i];
			return true;
		}
	return false;
}

void _cdecl GodObjectEnsureExistence(int id)
{
	if (id >= World.objects.num)
		return;
	if (!World.objects[id]){
		// reload...
	}
}

// mode: -1 = all...
int GodFindObjects(vector &pos, float radius, int mode, Array<Object*> &a)
{
	//bool mask_passive = (mode >= 0);
	//bool mask_active = (

	// well... until scripts can initialize local super arrays... m(-_-)m
	//a.init(sizeof(Object*));
	//     \(^_^)/   now they can!
	
	// old method.... better use octree...
	for (int i=0;i<World.objects.num;i++)
		if (World.objects[i]){
			if (_vec_length_fuzzy_(pos - World.objects[i]->pos) < radius)
				a.add(World.objects[i]);
		}
	return a.num;
}

typedef void object_callback_func(Object*);

Object *GodCreateObject(const string &filename, const string &name, const vector &pos, const quaternion &ang, int w_index)
{
	if (Engine.ResettingGame){
		msg_error("CreateObject during game reset");
		return NULL;
	}
	msg_db_f("GodCreateObject", 2);
	//msg_write(on);
	Model *m = LoadModelX(filename, false);
	if (!m)
		return NULL;

	Object *o = (Object*)m;
	m->name = name;
	m->pos = pos;
	m->ang = ang;
	o->UpdateMatrix();
	o->UpdateTheta();


	GodRegisterObject(m, w_index);

	m->OnInit();

	AddNetMsg(NET_MSG_CREATE_OBJECT, m->object_id, filename);

	return o;
}


Object *_cdecl _CreateObject(const string &filename, const vector &pos)
{
	return GodCreateObject(filename, filename, pos, q_id);
}

void AddNewForceField(vector pos,vector dir,int kind,int shape,float r,float v,float a,bool visible,float t)
{
	World.ForceField[World.NumForceFields]=new GodForceField;
	World.ForceField[World.NumForceFields]->Kind=kind;
	World.ForceField[World.NumForceFields]->Pos=pos;
	World.ForceField[World.NumForceFields]->Dir=dir;
	World.ForceField[World.NumForceFields]->Radius=r;
	World.ForceField[World.NumForceFields]->Vel=v;
	World.ForceField[World.NumForceFields]->Acc=a;
	World.ForceField[World.NumForceFields]->Visible=visible;
	World.ForceField[World.NumForceFields]->TimeToLife=t;
	World.NumForceFields++;
}

void DoForceFields()
{
	/*for (int f=0;f<NumForceFields;f++){
		if (ForceField[f]->TimeToLife<0){
			delete(ForceField[f]);
			NumForceFields--;
			for (int f2=f;f2<NumForceFields;f2++)
				ForceField[f2]=ForceField[f2+1];
			f--;
			continue;
		}

		// die eigendlichen Berechnungen
		ForceField[f]->TimeToLife -= Engine.Elapsed;
		ForceField[f]->Radius += Engine.Elapsed * ForceField[f]->Vel;
		for (int o=0;o<Objects.num;o++)
			if (Objects[o]->active_physics)
				if (Objects[o]->pos.bounding_cube(ForceField[f]->Pos, ForceField[f]->Radius)){
					float d = (Objects[o]->pos - ForceField[f]->Pos).length();
					if (d < ForceField[f]->Radius){
						vector n = Objects[o]->pos - ForceField[f]->Pos;
						float d = n.length();
						n /= d;
						if (ForceField[f]->Kind == FFKindRadialConst)
							Objects[o]->vel += ForceField[f]->Acc*Engine.Elapsed*n;
						if (ForceField[f]->Kind == FFKindRadialLinear)
							Objects[o]->vel += ForceField[f]->Acc*Engine.Elapsed*n*(ForceField[f]->Radius-d)/ForceField[f]->Radius;
						if (ForceField[f]->Kind == FFKindDirectionalConst)
							Objects[o]->vel += ForceField[f]->Acc*Engine.Elapsed*ForceField[f]->Dir;
						if (ForceField[f]->Kind == FFKindDirectionalLinear)
							Objects[o]->vel += ForceField[f]->Acc*Engine.Elapsed*ForceField[f]->Dir*(ForceField[f]->Radius-d)/ForceField[f]->Radius;
					}
				}
	}*/
}

void DoSounds()
{
#if 0
	sMusicField *MusicFieldLast=MusicFieldCurrent;
	MusicFieldCurrent=&MusicFieldGlobal;
	for (int i=0;i<NumMusicFields;i++)
		if (VecBetween(cam->Pos,MusicField[i].PosMin,MusicField[i].PosMax))
			MusicFieldCurrent=&MusicField[i];

	//if (!MusicEnabled)
	//	return;

	bool ChooseNew=false;

	if (NixSoundEnded(MusicCurrent))
		ChooseNew=true;
	if (MusicFieldCurrent!=MusicFieldLast)
		ChooseNew=true;
	if (MusicCurrent<0)
		ChooseNew=true;

	// neue Musik laden
	if (ChooseNew){
		if (MusicCurrent>=0)
			NixSoundStop(MusicCurrent);
		if (MusicFieldCurrent->NumMusicFiles>0){
			int n=int(float(rand())*float(MusicFieldCurrent->NumMusicFiles)/float(RAND_MAX));
			MusicCurrent=MetaLoadSound(MusicFieldCurrent->MusicFile[n]);
			NixSoundPlay(MusicCurrent,false);
		}
	}
#endif
}

void SetSoundState(bool paused,float scale,bool kill,bool restart)
{
#if 0
	if (kill){
		NixSoundStop(MusicCurrent);
	}
	if (restart){
		NixSoundPlay(MusicCurrent,false);
	}
	/*if (!meta->MusicEnabled)
		return;*/

	MetaListenerRate=paused?0:scale;

	vector nv=v_0;
	if (paused)
		SoundSetListener(nv,nv,nv);
#endif
}

vector _cdecl GetG(vector &pos)
{
	// homogener, konstanter Anteil (Welt)
	vector g=World.gravity;
/*		// radiale Kraftfelder
		for (int i=0;i<FxNumForceFields;i++)
			if (FxForceField[i]->Used)
				if (fx->ForceField[i]->Enabled)
					if (VecBoundingBox(pos,fx->ForceField[i]->Pos,fx->ForceField[i]->RadiusMax)){
						float r=VecLength(fx->ForceField[i]->Pos-pos);
						if ((r>fx->ForceField[i]->RadiusMin)&&(r<fx->ForceField[i]->RadiusMax)){
							vector dir=(fx->ForceField[i]->Pos-pos)/r;
							if (fx->ForceField[i]->InvQuadratic)
								g+= dir * (10000.0f/(r*r))*fx->ForceField[i]->Strength;
							else
								g+= dir * (1-r/fx->ForceField[i]->RadiusMax)*fx->ForceField[i]->Strength;
						}
					}*/
	return g;
}



#ifdef USE_ODE

void PhysicsDataToODE()
{
	msg_db_f("PhysicsDataToODE", 3);
	// data.. x -> ode
	foreach(Model *o, World.objects){
		if (o){
			dBodyID b = (dBodyID)o->body_id;
			if (b != 0){
				dBodyAddForce(b, o->force_ext.x, o->force_ext.y, o->force_ext.z);
				dBodyAddTorque(b, o->torque_ext.x, o->torque_ext.y, o->torque_ext.z);
				dBodySetPosition(b, o->pos.x, o->pos.y, o->pos.z);
				dBodySetLinearVel(b, o->vel.x, o->vel.y, o->vel.z);
				dBodySetAngularVel(b, o->rot.x, o->rot.y, o->rot.z);
				dQuaternion qq;
				qx2ode(&o->ang, qq);
				dBodySetQuaternion(b, qq);
			}
		}
	}
}

void PhysicsDataFromODE()
{
	msg_db_f("PhysicsDataFromODE", 3);
	// data.. ode -> x
	foreach(Object *o, World.objects){
		if (o){
			dBodyID b = (dBodyID)o->body_id;
			if (b != 0){
				o->pos = *(vector*)dBodyGetPosition(b);
				o->vel = *(vector*)dBodyGetLinearVel(b);
				o->rot = *(vector*)dBodyGetAngularVel(b);
				qode2x(dBodyGetQuaternion(b), &o->ang);
				o->UpdateMatrix();
				o->UpdateTheta();
				o->_ResetPhysAbsolute_();
			}
		}
	}
}

#endif

void GodDoCollisionDetection()
{
#ifdef _X_ALLOW_X_
	msg_db_f("GodDoCollisionDetection", 2);
#ifdef _X_ALLOW_PHYSICS_DEBUG_
	PhysicsDebugColData.Num = 0;
#endif
	
	// object <-> terrain
	msg_db_m("---T4G",4);
	for (int i=0;i<World.objects.num;i++)
		if (World.objects[i])
			if (World.objects[i]->active_physics)
				if (!World.objects[i]->frozen)
					Test4Ground(World.objects[i]);

	TestObjectSanity("God::Coll   1");

	// object <-> object
	msg_db_m("---T4O",4);
	for (int i=0;i<World.objects.num;i++)
		if (World.objects[i])
			for (int j=i+1;j<World.objects.num;j++)
				if (World.objects[j])
					if ((!World.objects[i]->frozen) || (!World.objects[j]->frozen))
						if (!ObjectsLinked(World.objects[i], World.objects[j]))
							Test4Object(World.objects[i],World.objects[j]);

	TestObjectSanity("God::Coll   2");
	
	DoCollisionObservers();

#ifdef _X_ALLOW_PHYSICS_DEBUG_
	PhysicsTimeCol += HuiGetTime(PhysicsTimer);
#endif
#endif
}

void ApplyGravity()
{
	msg_db_m("--G",3);
	for (int i=0;i<World.objects.num;i++)
		if (World.objects[i])
			if (!World.objects[i]->frozen){
				float ttf = World.objects[i]->time_till_freeze;
				vector g = World.objects[i]->mass * World.objects[i]->g_factor * World.gravity;
				World.objects[i]->AddForce(g, v_0);
				//                                                     GetG(World.objects[i]->Pos));
				World.objects[i]->time_till_freeze = ttf;
			}
}

void ResetExternalForces()
{
	for (int i=0;i<World.objects.num;i++)
		if (World.objects[i])
			World.objects[i]->force_ext = World.objects[i]->torque_ext = v_0;
}


void GodCalcMove()
{
	if (Engine.Elapsed == 0)
		return;

	msg_db_f("GodCalcMove",2);
	//CreateObjectLists();
	Engine.NumRealColTests = 0;

#ifdef _X_ALLOW_PHYSICS_DEBUG_
	HuiGetTime(PhysicsTimer);
#endif

	TestObjectSanity("God::CM prae");
	bool phys_en = Engine.PhysicsEnabled;// && (!Engine.FirstFrame);
	bool coll_en = Engine.CollisionsEnabled;// && (!Engine.FirstFrame);

	// no physics?
	if (!phys_en){
		for (int i=0;i<World.objects.num;i++)
			if (World.objects[i])
				World.objects[i]->UpdateMatrix();
		ResetExternalForces();
		return;
	}else if (!coll_en){
		for (int i=0;i<World.objects.num;i++)
			if (World.objects[i])
				//if (!World.objects[i]->Frozen)
					World.objects[i]->DoPhysics();
		ResetExternalForces();
		return;
	}

	// force fields
	//DoForceFields();

	// add gravitation
	ApplyGravity();
	

	// do the physics several times to increase precision!
	float elapsed_temp = Engine.Elapsed;
	Engine.Elapsed /= (float)PhysicsNumSteps;
	for (int ttt=0;ttt<PhysicsNumSteps;ttt++){

#ifdef USE_ODE
		PhysicsDataToODE();
#endif

		TestObjectSanity("God::CM prae Links");

#ifdef _X_ALLOW_PHYSICS_DEBUG_
		PhysicsTimePhysics += HuiGetTime(PhysicsTimer);
#endif

		// statics and hinges...
		msg_db_m("--L",3);
#ifdef _X_ALLOW_X_
		DoLinks(PhysicsNumLinkSteps);
#endif

		TestObjectSanity("God::CM post Links");

#ifndef USE_ODE
		// propagate through space
		msg_db_m("--P",3);
		for (int i=0;i<World.objects.num;i++)
			if (World.objects[i])
				//if (!World.objects[i]->Frozen)
					World.objects[i]->DoPhysics();
#endif


		TestObjectSanity("God::CM   1");

#ifdef _X_ALLOW_PHYSICS_DEBUG_
		PhysicsTimePhysics += HuiGetTime(PhysicsTimer);
#endif

		// collision detection
		GodDoCollisionDetection();

#ifdef USE_ODE
		// physics...
		dWorldQuickStep(world_id, Engine.Elapsed);
		dJointGroupEmpty(contactgroup);
		PhysicsDataFromODE();
#endif

		TestObjectSanity("God::CM   2");


	}
	Engine.Elapsed = elapsed_temp;

	ResetExternalForces();

#if 0
	// mainly model animation
	for (int i=0;i<NumObjects;i++)
		if (ObjectExisting[i])
			Objects[i]->CalcMove();
#endif


#ifdef _X_ALLOW_PHYSICS_DEBUG_
	PhysicsTimePhysics += HuiGetTime(PhysicsTimer);
#endif
}

void GodCalcMove2()
{
	// mainly model animation
	for (int i=0;i<World.objects.num;i++)
		if (World.objects[i])
			World.objects[i]->CalcMove(Engine.Elapsed);
}

typedef void on_collide_object_func(Object*,Object*);
typedef void on_collide_terrain_func(Object*,Terrain*);

void Test4Ground(Object *o)
{
#ifdef _X_ALLOW_X_
	for (int i=0;i<World.terrains.num;i++){
		if (CollideObjectTerrain(o, World.terrains[i])){
			//msg->Write(string2("   col %d <-> terrain %d",i,j));

			if (inf_v(o->vel))
				msg_error("inf   Test4Ground Vel 1");

			// script callback function
			o->OnCollideT(World.terrains[i]);

			World.terrain_object->SetMaterial(World.terrains[i]->material, SET_MATERIAL_FRICTION);
			World.terrain_object->object_id = i + 0x40000000; //(1<<30);
			ColData.o2 = World.terrain_object;
			
			// physical reaction
#ifdef _X_ALLOW_PHYSICS_DEBUG_
			if (Elapsed > 0)
#endif
			HandleCollision();

	// debugging
	if (inf_v(o->vel)){
		msg_error("inf   Test4Ground Vel 2");
		msg_write(o->name);
		msg_write(format("num = %d",ColData.num));
		for (int j=0;j<ColData.num;j++){
			msg_write(format("n = (%f  %f  %f)", ColData.normal[j].x, ColData.normal[j].y, ColData.normal[j].z));
			msg_write(format("p = (%f  %f  %f)", ColData.pos[j].x, ColData.pos[j].y, ColData.pos[j].z));
			msg_write(format("depth = %f", ColData.depth));
		}
	}
		}
	}
#endif
}

void Test4Object(Object *o1,Object *o2)
{
#ifdef _X_ALLOW_X_
	msg_db_f("Test4Object",5);

	// Kollision?
	if (!CollideObjects(o1, o2))
		return;

	// debugging
	if (inf_v(o1->vel))
			msg_error("inf   Test4Object Vel 1   (1)");
	if (inf_v(o2->vel))
			msg_error("inf   Test4Object Vel 1   (2)");

	// script callback functions
	o1->OnCollideM(o2);
	o2->OnCollideM(o1);

	// physical reaction
#ifdef _X_ALLOW_PHYSICS_DEBUG_
	if (Elapsed > 0)
#endif	
	HandleCollision();
	

	// debugging
	if (inf_v(o1->vel)){
		msg_error("inf   Test4Object Vel 2    (1)");
		msg_write(o1->name);
		msg_write(o2->name);
		msg_write(format("num = %d", ColData.num));
		for (int j=0;j<ColData.num;j++){
			msg_write(format("n = (%f  %f  %f)", ColData.normal[j].x, ColData.normal[j].y, ColData.normal[j].z));
			msg_write(format("p = (%f  %f  %f)", ColData.pos[j].x, ColData.pos[j].y, ColData.pos[j].z));
			msg_write(format("depth = %f", ColData.depth));
		}
	}
	if (inf_v(o2->vel)){
		msg_error("inf   Test4Object Vel 2    (2)");
		msg_write(o1->name);
		msg_write(o2->name);
		msg_write(format("num = %d", ColData.num));
		for (int j=0;j<ColData.num;j++){
			msg_write(format("n = (%f  %f  %f)", ColData.normal[j].x, ColData.normal[j].y, ColData.normal[j].z));
			msg_write(format("p = (%f  %f  %f)", ColData.pos[j].x, ColData.pos[j].y, ColData.pos[j].z));
			msg_write(format("depth = %f", ColData.depth[j]));
		}
	}
#endif
}

bool GodTrace(const vector &p1, const vector &p2, TraceData &data, bool simple_test, Model *o_ignore)
{
	msg_db_f("GodTrace",4);
	vector dir = p2 - p1;
	float range = dir.length();
	dir /= range;
	float dmin = range;
	data.type = TRACE_TYPE_NONE;
	for (int i=0;i<World.terrains.num;i++){
		if (World.terrains[i]->Trace(p1, p2, dir, dmin, data, simple_test)){
			if (simple_test)
				return true;
			float d = (p1 - data.point).length();
			if (d < dmin)
				dmin = d;
		}
	}
	for (int i=0;i<World.objects.num;i++)
		if (World.objects[i]){
			if (World.objects[i] == o_ignore)
				continue;
			vector p2t = p1 + dir * dmin;
			if (World.objects[i]->Trace(p1, p2t, dir, dmin, data, simple_test)){
				if (simple_test)
					return true;
				float d = (p1 - data.point).length();
				if (d < dmin){
					dmin = d;
					data.object = World.objects[i];
				}
			}
		}
	return (dmin < range);
}

// do everything needed before drawing the objects
void GodPreDraw(vector &cam_pos)
{
	msg_db_f("GodPreDraw",2);
	World.add_all_objects_to_lists = false;

// sort models by depth
	/*MetaClearSorted();
	// objects
	msg_db_m("--O",3);
	for (int i=0;i<NumObjects;i++)
		if (ObjectExisting[i])
			Objects[i]->Draw(cam_pos);*/
}

void GodDrawSorted();

// actually draw the objects
void GodDraw()
{
	msg_db_f("GodDraw",3);
	// draw the sorted models
	//MetaDrawSorted();
	GodDrawSorted();
	msg_db_m("b",3);

	// force fields ....(obsolete?!)
#ifdef _X_ALLOW_X_
	NixSetAlpha(AlphaSourceAlpha,AlphaOne);
	NixSetCull(CullNone);
	NixSetZ(false,true);
	for (int i=0;i<World.NumForceFields;i++)
		if (World.ForceField[i]->Visible){
			color c=color(World.ForceField[i]->TimeToLife/4,1,1,1);
			NixSetMaterial(c,c,Black,0,Black);
			//Fx::DrawBall(ForceField[i]->Pos,ForceField[i]->Radius,8,16);
		}
	NixSetZ(true,true);
	NixSetCull(CullDefault);
	NixSetAlpha(AlphaNone);
#endif
}

void GodRegisterObject(Model *o, int index)
{
	msg_db_f("GodRegisterObject", 2);
	int on = index;
	if (on < 0){
		// ..... better use a list of "empty" objects???
		for (int i=GodNumReservedObjects;i<World.objects.num;i++)
			if (!World.objects[i])
				on = i;
	}else{
		if (on >= World.objects.num)
			World.objects.resize(on+1);
		if (World.objects[on]){
			msg_error("CreateObject:  object index already in use " + i2s(on));
			return;
		}
	}
	if (on < 0){
		on = World.objects.num;
		World.objects.add(NULL);
	}
	World.objects[on] = (Object*)o;

	GodRegisterModel(o);

	o->object_id = on;
//	strcpy(ObjectFilename[on], filename);

#ifdef USE_ODE
	if (o->active_physics){
		o->body_id = dBodyCreate(world_id);
		dBodySetPosition((dBodyID)o->body_id, o->pos.x, o->pos.y, o->pos.z);
		dMass m;
		dMassSetParameters(&m, o->mass, 0, 0, 0, o->theta._00, o->theta._11, o->theta._22, o->theta._01, o->theta._02, o->theta._12);
		dBodySetMass((dBodyID)o->body_id, &m);
		dBodySetData((dBodyID)o->body_id, o);
		dQuaternion qq;
		qx2ode(&o->ang, qq);
		dBodySetQuaternion((dBodyID)o->body_id, qq);
	}else
		o->body_id = 0;

	vector d = o->max - o->min;
	o->geom_id = dCreateBox(space_id, d.x, d.y, d.z); //dCreateSphere(0, 1); // space, radius
//	msg_write((int)o->geom_id);
	dGeomSetBody((dGeomID)o->geom_id, (dBodyID)o->body_id);
#endif
}



// un-object a model
void GodUnregisterObject(Model *m)
{
	if (m->object_id < 0)
		return;
	msg_db_f("GodUnregisterObject", 2);

#ifdef USE_ODE
	if (m->body_id != 0)
		dBodyDestroy((dBodyID)m->body_id);
	m->body_id = 0;

	if (m->geom_id != 0)
		dGeomDestroy((dGeomID)m->geom_id);
	m->geom_id = 0;
#endif

	// ego...
	if (m == World.ego)
		World.ego = NULL;

	AddNetMsg(NET_MSG_DELETE_OBJECT, m->object_id, "");

	// remove from list
	World.objects[m->object_id] = NULL;
	m->object_id = -1;
}

// add a model to the (possible) rendering list
void GodRegisterModel(Model *m)
{
	if (m->registered)
		return;
	msg_db_f("GodRegisterModel", 2);
	
	for (int i=0;i<m->material.num;i++){
		Material *mat = &m->material[i];
		bool trans = !mat->alpha_z_buffer; //false;
		/*if (mat->TransparencyMode>0){
			if (mat->TransparencyMode == TransparencyModeFunctions)
				trans = true;
			if (mat->TransparencyMode == TransparencyModeFactor)
				trans = true;
		}*/

		PartialModel p;
		p.model = m;
		p.material = mat;
		p.mat_index = i;
		p.transparent = trans;
		p.shadow = false;
		if (trans)
			SortedTrans.add(p);
	    else
			SortedNonTrans.add(p);
	}

#ifdef _X_ALLOW_X_
	for (int i=0;i<m->fx.num;i++)
		if (m->fx[i])
			m->fx[i]->Enable(true);
#endif
	
	m->registered = true;
	
	// sub models
	for (int i=0;i<m->bone.num;i++)
		if (m->bone[i].model)
			GodRegisterModel(m->bone[i].model);
}

// remove a model from the (possible) rendering list
void GodUnregisterModel(Model *m)
{
	if (!m->registered)
		return;
	msg_db_f("GodUnregisterModel", 2);
	//printf("%p   %s\n", m, MetaGetModelFilename(m));
	
	for (int i=SortedTrans.num-1;i>=0;i--)
		if (SortedTrans[i].model == m)
			SortedTrans.erase(i);
	for (int i=SortedNonTrans.num-1;i>=0;i--)
		if (SortedNonTrans[i].model == m)
			SortedNonTrans.erase(i);

#ifdef _X_ALLOW_X_
	if (!Engine.ResettingGame)
		for (int i=0;i<m->fx.num;i++)
			if (m->fx[i])
				m->fx[i]->Enable(false);
#endif
	
	m->registered = false;
	//msg_db_m("med",2);
	//printf("%d\n", m->NumBones);

	// sub models
	for (int i=0;i<m->bone.num;i++)
		if (m->bone[i].model)
			GodUnregisterModel(m->bone[i].model);
}

void WorldShiftAll(const vector &dpos)
{
	foreach(Object *o, World.objects)
		if (o)
			o->pos += dpos;
	foreach(Terrain *t, World.terrains)
		t->pos += dpos;
#ifdef _X_ALLOW_X_
	Fx::ShiftAll(dpos);
	Light::ShiftAll(dpos);
#endif
	CameraShiftAll(dpos);
}

inline bool is_ignored(Model *m)
{
	for (int i=0;i<cur_cam->ignore.num;i++)
		if (m == cur_cam->ignore[i])
			return true;
	return false;
}

inline void fill_pmv(vector &pos, Array<PartialModel> &p, Array<PartialModelView> &vp, bool do_sort, bool sort_inv)
{
	vp.clear();
	for (int i=0;i<p.num;i++){
		Model *m = p[i].model;
		vector dpos = pos - m->pos;
		
		int detail = SKIN_HIGH;
		float dist = _vec_length_(dpos); // real distance to the camera
		float dist_2 = dist * Engine.DetailFactorInv; // more adequate distance value

		// ignore?
		if (is_ignored(m))
			continue;

		// which level of detail?
		if (dist_2 > m->detail_dist[2]){		detail = -1;	continue;	}
		else if (dist_2 > m->detail_dist[1])	detail = SKIN_LOW;
		else if (dist_2 > m->detail_dist[0])	detail = SKIN_MEDIUM;

		PartialModelView pmv;
		pmv.p = &p[i];
		pmv.z = dist;
		pmv.detail = detail;
		vp.add(pmv);

		// shadows...
#ifdef _X_ALLOW_X_
		if ((Engine.ShadowLevel > 0) && (detail == SKIN_HIGH) && (p[i].mat_index == 0) && (m->allow_shadow)){
			int sd = Engine.ShadowLowerDetail ? SKIN_MEDIUM : SKIN_HIGH;
			if (m->skin[sd]->sub[p[i].mat_index].num_triangles > 0)
				Fx::AddShadow(m, sd);
		}
#endif
	}
	if ((do_sort) && (vp.num > 0)){
		// sorting (FAR ones first
		std::sort(&vp[0], &vp[vp.num]);
		if (sort_inv)
			// sorting (NEAR ones first)
			std::reverse(&vp[0], &vp[vp.num]);
	}
}

inline void draw_pmv(Array<PartialModelView> &vp)
{
	// camera frustrum data
	vector pos = cur_cam->pos;
	vector dir = cur_cam->ang * e_z;
	quaternion a2;
	a2 = cur_cam->ang * quaternion(vector(0, +0.9f, 0));
	vector dir_l = a2 * e_z;
	a2 = cur_cam->ang * quaternion(vector(0, -0.9f, 0));
	vector dir_r = a2 * e_z;
	a2 = cur_cam->ang * quaternion(vector(+1.0f, 0, 0));
	vector dir_t = a2 * e_z;
	a2 = cur_cam->ang * quaternion(vector(-1.0f, 0, 0));
	vector dir_b = a2 * e_z;
	
	for (int i=0;i<vp.num;i++){
		PartialModel *p = (PartialModel*)vp[i].p;
		Model *m = p->model;
		vector dpos = pos - m->pos;

		// camera frustrum testing
		if (dpos * dir > m->radius)
			continue;
		if (dpos * dir_l > m->radius)
			continue;
		if (dpos * dir_r > m->radius)
			continue;
		if (dpos * dir_t > m->radius)
			continue;
		if (dpos * dir_b > m->radius)
			continue;

#ifdef _X_ALLOW_X_
		Light::Apply(m->pos);
#endif

		// draw!
		p->material->apply();
		//m->Draw(0, m->_matrix, true, false);//p->shadow);
		m->JustDraw(p->mat_index, vp[i].detail);
	}
	NixSetShader(NULL);
}

static int fill_frame = 100;

int ffframe = 0;

void GroupDuplicates()
{
	for (int i=0;i<World.objects.num;i++){}
}

void GodDrawSorted()
{
	msg_db_f("GodDrawSorted",2);

	ffframe ++;
	if ((ffframe % 100) == 0)
		GroupDuplicates();

// fill data structures
	//fill_frame ++;
	//if (fill_frame > 10){
		// TODO: refill when SortedNonTrans or SortedNonTrans change... pointers...vector allocation...
		vector pos = cur_cam->pos;
		fill_pmv(pos, SortedNonTrans, cur_cam->pmvd.opaque, Engine.SortingEnabled, true);
		fill_pmv(pos, SortedTrans, cur_cam->pmvd.trans, true, false);
		fill_frame = 0;
	//}

// non-transparent models
	// overlapping each other
	if (Engine.ZBufferEnabled)
		NixSetZ(true,true);
	else
		NixSetZ(false,false);

	// drawing
	draw_pmv(cur_cam->pmvd.opaque);

//transparent models
	// test but don't write
	if (Engine.ZBufferEnabled)
		NixSetZ(false,true);

	// drawing
	draw_pmv(cur_cam->pmvd.trans);

	
	// reset the z buffer
	NixSetZ(true,true);
}
