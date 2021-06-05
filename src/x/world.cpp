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
#include "world.h"
#include "../lib/file/file.h"
//#include "../lib/vulkan/vulkan.h"
#include "../lib/nix/nix.h"
#include "../meta.h"
#include "object.h"
#include "model.h"
#include "ModelManager.h"
#include "Material.h"
#include "terrain.h"

#include "../lib/xfile/xml.h"

#ifdef _X_ALLOW_X_
#include "../fx/Light.h"
#include "../fx/Particle.h"
#endif

#if 0
#include "model_manager.h"
#include "../lib/nix/nix.h"
#ifdef _X_ALLOW_X_
#include "../physics/physics.h"
#include "../physics/links.h"
#include "../physics/collision.h"
#include "../fx/fx.h"
#endif
#include "../networking.h"
#endif
#include "camera.h"


nix::Texture *tex_white = nullptr;
nix::Texture *tex_black = nullptr;


//vulkan::DescriptorSet *rp_create_dset(const Array<vulkan::Texture*> &tex, vulkan::UniformBuffer *ubo);



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


#ifdef _X_ALLOW_PHYSICS_DEBUG_
	int PhysicsTimer;
	float PhysicsTimeCol, PhysicsTimePhysics, PhysicsTimeLinks;
	bool PhysicsStopOnCollision = false;
#endif



// game data
World world;


#ifdef _X_ALLOW_X_
void DrawSplashScreen(const string &str, float per);
void ScriptingObjectInit(Object *o);
#else
void DrawSplashScreen(const string &str, float per){}
void ScriptingObjectInit(Object *o){}
#endif


// network messages
void AddNetMsg(int msg, int argi0, const string &args)
{
#if 0
#ifdef _X_ALLOW_X_
	if ((!world.net_msg_enabled) || (!Net.Enabled))
		return;
	GodNetMessage m;
	m.msg = msg;
	m.arg_i[0] = argi0;
	m.arg_s = args;
	world.net_messages.add(m);
#endif
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







void GodInit() {
//	world.ubo_light = new vulkan::UniformBuffer(sizeof(UBOLight), 64);
//	world.ubo_fog = new vulkan::UniformBuffer(64);

	Image im;
	tex_white = new nix::Texture();
	tex_black = new nix::Texture();
	im.create(16, 16, White);
	tex_white->overwrite(im);
	im.create(16, 16, Black);
	tex_black->overwrite(im);

	world.reset();

	world.terrain_object = new Object();
	world.terrain_object->update_matrix();

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

void GodEnd() {
//	delete world.ubo_light;
//	delete world.ubo_fog;
}

World::World() {
//	ubo_light = nullptr;
//	ubo_fog = nullptr;

	//world.particle_manager = new ParticleManager();

	reset();
}

void World::reset() {
	net_msg_enabled = false;
	net_messages.clear();

	gravity = v_0;

	// terrains
	for (auto *t: terrains)
		delete t;
	terrains.clear();

	// objects
	for (auto *o: objects)
		if (o)
			delete o;//unregister_object(o); // actual deleting done by ModelManager
	objects.clear();
	num_reserved_objects = 0;
	
	for (auto &s: sorted_trans)
		s.clear();
	sorted_trans.clear();
	for (auto &s: sorted_opaque)
		s.clear();
	sorted_opaque.clear();

	/*for (auto *l: lights)
		delete l;
	lights.clear();

	particle_manager->clear();*/



	// music
	/*if (meta->MusicEnabled){
		NixSoundStop(MusicCurrent);
	}*/

	// skybox
	//   (models deleted by meta)
	skybox.clear();
	

	// initial data for empty world...
	fog._color = White;
	fog.mode = 0;//nix::FogMode::EXP;
	fog.distance = 10000;
	fog.enabled = false;
	fog.start = 0;
	fog.end = 100000;
	speed_of_sound = 1000;
	
	engine.physics_enabled = false;
	engine.collisions_enabled = true;
	physics_num_steps = 10;
	physics_num_link_steps = 5;


	// physics
#ifdef _X_ALLOW_X_
	//LinksReset();
#endif
#ifdef USE_ODE
	if (ode_world_created){
		dWorldDestroy(world_id);
		dSpaceDestroy(space_id);
	}else{
		dInitODE();
	}
	world_id = dWorldCreate();
	space_id = dSimpleSpaceCreate(0);
	//space_id = dHashSpaceCreate(0);
	/*int m1, m2;
	dHashSpaceGetLevels(space_id, &m1, &m2);
	printf("hash:    %d  %d\n", m1, m2);*/
	ode_world_created = true;
	world.terrain_object->body_id = 0;
	world.terrain_object->geom_id = dCreateSphere(0, 1); // space, radius
	dGeomSetBody((dGeomID)world.terrain_object->geom_id, (dBodyID)world.terrain_object->body_id);
#endif
}

void LevelData::reset() {
	world_filename = "";
	terrains.clear();
	objects.clear();
	skybox_filename.clear();
	skybox_ang.clear();
	scripts.clear();

	ego_index = -1;
	background_color = Gray;
	lights.clear();

	gravity = v_0;
	fog.enabled = false;
}

bool World::load(const LevelData &ld) {
	net_msg_enabled = false;
	bool ok = true;
	reset();


	engine.physics_enabled = ld.physics_enabled;
	engine.collisions_enabled = true;//LevelData.physics_enabled;
	gravity = ld.gravity;
	fog = ld.fog;

#ifdef _X_ALLOW_X_
	for (auto &l: ld.lights) {
		auto *ll = new Light(l.pos, l.ang.ang2dir(), l._color, l.radius, -1);
		ll->harshness = l.harshness;
		ll->enabled = l.enabled;
		add_light(ll);
	}
#endif

	// skybox
	skybox.resize(ld.skybox_filename.num);
	for (int i=0; i<skybox.num; i++){
		skybox[i] = ModelManager::load(ld.skybox_filename[i]);
		if (skybox[i])
			skybox[i]->ang = quaternion::rotation_v(ld.skybox_ang[i]);
	}
	background = ld.background_color;

	for (auto &c: ld.cameras) {
		cam->pos = c.pos;
		cam->ang = quaternion::rotation(c.ang);
		cam->min_depth = c.min_depth;
		cam->max_depth = c.max_depth;
		cam->exposure = c.exposure;
		cam->fov = c.fov;
		break;
	}

	// objects
	ego = NULL;
	objects.clear(); // make sure the "missing" objects are NULL
	objects.resize(ld.objects.num);
	num_reserved_objects = ld.objects.num;
	foreachi(auto &o, ld.objects, i)
		if (!o.filename.is_empty()){
			auto q = quaternion::rotation(o.ang);
			Object *oo = create_object(o.filename, o.name, o.pos, q, i);
			ok &= (oo >= 0);
			if (oo){
				oo->vel = o.vel;
				oo->rot = o.rot;
			}
			if (ld.ego_index == i)
				ego = oo;
			if (i % 5 == 0)
				DrawSplashScreen("Objects", (float)i / (float)ld.objects.num / 5 * 3);
		}
	add_all_objects_to_lists = true;

	// terrains
	foreachi(auto &t, ld.terrains, i){
		DrawSplashScreen("Terrain...", 0.6f + (float)i / (float)ld.terrains.num * 0.4f);
		Terrain *tt = create_terrain(t.filename, t.pos);
		ok &= !tt->error;
	}

	scripts = ld.scripts;

	net_msg_enabled = true;
	return ok;
}

Terrain *World::create_terrain(const Path &filename, const vector &pos) {
	Terrain *tt = new Terrain(filename, pos);

//	tt->ubo = new vulkan::UniformBuffer(64*3);
//	tt->dset = rp_create_dset(tt->material->textures, tt->ubo);
	terrains.add(tt);
	return tt;
}

static vector s2v(const string &s) {
	auto x = s.explode(" ");
	return vector(x[0]._float(), x[1]._float(), x[2]._float());
}

// RGBA
static color s2c(const string &s) {
	auto x = s.explode(" ");
	return color(x[3]._float(), x[0]._float(), x[1]._float(), x[2]._float());
}

bool LevelData::load(const Path &filename) {
	world_filename = filename;

	xml::Parser p;
	p.load(engine.map_dir << filename.with(".world"));
	auto *meta = p.elements[0].find("meta");
	if (meta) {
		for (auto &e: meta->elements) {
			if (e.tag == "background") {
				background_color = s2c(e.value("color"));
			} else if (e.tag == "skybox") {
				skybox_filename.add(e.value("file"));
				skybox_ang.add(v_0);
			} else if (e.tag == "physics") {
				physics_enabled = e.value("enabled")._bool();
				gravity = s2v(e.value("gravity"));
			} else if (e.tag == "fog") {
				fog.enabled = e.value("enabled")._bool();
				fog.mode = e.value("mode")._int();
				fog.start = e.value("start")._float();
				fog.end = e.value("end")._float();
				fog.distance = 1.0f / e.value("density")._float();
				fog._color = s2c(e.value("color"));
			} else if (e.tag == "script") {
				LevelDataScript s;
				s.filename = e.value("file");
				for (auto &ee: e.elements) {
					TemplateDataScriptVariable v;
					v.name = ee.value("name").lower().replace("_", "");
					v.value = ee.value("value");
					s.variables.add(v);
				}
				scripts.add(s);
			}
		}
	}


	auto *cont = p.elements[0].find("3d");
	if (cont) {
		for (auto &e: cont->elements) {
			if (e.tag == "camera") {
				LevelDataCamera c;
				c.pos = s2v(e.value("pos"));
				c.ang = s2v(e.value("ang"));
				c.fov = e.value("fov", f2s(pi/4, 3))._float();
				c.min_depth = e.value("minDepth", "1")._float();
				c.max_depth = e.value("maxDepth", "10000")._float();
				c.exposure = e.value("exposure", "1")._float();
				cameras.add(c);
			} else if (e.tag == "light") {
				LevelDataLight l;
				l.radius = e.value("radius")._float();
				l.harshness = e.value("harshness")._float();
				l._color = s2c(e.value("color"));
				l.ang = s2v(e.value("ang"));
				if (e.value("type") == "directional")
					l.radius = -1;
				l.enabled = e.value("enabled", "true")._bool();
				lights.add(l);
			} else if (e.tag == "terrain") {
				LevelDataTerrain t;
				t.filename = e.value("file");
				t.pos = s2v(e.value("pos"));
				terrains.add(t);
			} else if (e.tag == "object") {
				LevelDataObject o;
				o.filename = e.value("file");
				o.name = e.value("name");
				o.pos = s2v(e.value("pos"));
				o.ang = s2v(e.value("ang"));
				o.vel = v_0;
				o.rot = v_0;
				objects.add(o);
			}
		}
	}

	return true;
}

bool GodLoadWorld(const Path &filename) {
	LevelData level_data;
	bool ok = level_data.load(filename);
	ok &= world.load(level_data);
	return ok;
}

#if 0
Object *GetObjectByName(const string &name)
{
	for (int i=0;i<world.objects.num;i++)
		if (world.objects[i])
			if (name == world.objects[i]->script_data.name){
				//msg_write(i);
				return world.objects[i];
			}
	msg_error(format("object \"%s\" not found!", name.c_str()));
	return NULL;
}

bool NextObject(Object **o)
{
	int id=-1;
	if (*o)
		id = (*o)->object_id;
	for (int i=id+1;i<world.objects.num;i++)
		if (world.objects[i]){
			*o = world.objects[i];
			return true;
		}
	return false;
}

void _cdecl GodObjectEnsureExistence(int id)
{
	if (id >= world.objects.num)
		return;
	if (!world.objects[id]){
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
	for (int i=0;i<world.objects.num;i++)
		if (world.objects[i]){
			if (_vec_length_fuzzy_(pos - world.objects[i]->pos) < radius)
				a.add(world.objects[i]);
		}
	return a.num;
}
#endif

Object *World::create_object(const Path &filename, const string &name, const vector &pos, const quaternion &ang, int w_index) {
	if (engine.resetting_game)
		throw Exception("CreateObject during game reset");

	if (filename.is_empty())
		throw Exception("CreateObject: empty filename");

	//msg_write(on);
	Model *m = ModelManager::load(filename);

	Object *o = (Object*)m;
	m->script_data.name = name;
	m->pos = pos;
	m->ang = ang;
	o->update_matrix();
	o->update_theta();


	register_object(m, w_index);

	m->on_init();

	AddNetMsg(NET_MSG_CREATE_OBJECT, m->object_id, filename.str());

	return o;
}

#if 0
void AddNewForceField(vector pos,vector dir,int kind,int shape,float r,float v,float a,bool visible,float t)
{
	world.ForceField[world.NumForceFields]=new GodForceField;
	world.ForceField[world.NumForceFields]->Kind=kind;
	world.ForceField[world.NumForceFields]->Pos=pos;
	world.ForceField[world.NumForceFields]->Dir=dir;
	world.ForceField[world.NumForceFields]->Radius=r;
	world.ForceField[world.NumForceFields]->Vel=v;
	world.ForceField[world.NumForceFields]->Acc=a;
	world.ForceField[world.NumForceFields]->Visible=visible;
	world.ForceField[world.NumForceFields]->TimeToLife=t;
	world.NumForceFields++;
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
		ForceField[f]->TimeToLife -= engine.Elapsed;
		ForceField[f]->Radius += engine.Elapsed * ForceField[f]->Vel;
		for (int o=0;o<Objects.num;o++)
			if (Objects[o]->physics_data.active)
				if (Objects[o]->pos.bounding_cube(ForceField[f]->Pos, ForceField[f]->Radius)){
					float d = (Objects[o]->pos - ForceField[f]->Pos).length();
					if (d < ForceField[f]->Radius){
						vector n = Objects[o]->pos - ForceField[f]->Pos;
						float d = n.length();
						n /= d;
						if (ForceField[f]->Kind == FFKindRadialConst)
							Objects[o]->vel += ForceField[f]->Acc*engine.Elapsed*n;
						if (ForceField[f]->Kind == FFKindRadialLinear)
							Objects[o]->vel += ForceField[f]->Acc*engine.Elapsed*n*(ForceField[f]->Radius-d)/ForceField[f]->Radius;
						if (ForceField[f]->Kind == FFKindDirectionalConst)
							Objects[o]->vel += ForceField[f]->Acc*engine.Elapsed*ForceField[f]->Dir;
						if (ForceField[f]->Kind == FFKindDirectionalLinear)
							Objects[o]->vel += ForceField[f]->Acc*engine.Elapsed*ForceField[f]->Dir*(ForceField[f]->Radius-d)/ForceField[f]->Radius;
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
	vector g=world.gravity;
/*		// radiale Kraftfelder
		for (int i=0;i<FxNumForceFields;i++)
			if (FxForceField[i]->Used)
				if (fx->ForceField[i]->Enabled)
					if (VecBoundingBox(pos,fx->ForceField[i]->Pos,fx->ForceField[i]->RadiusMax)){
						float r=VecLength(fx->ForceField[i]->Pos-pos);
						if ((r>fx->ForceField[i]->RadiusMin)and(r<fx->ForceField[i]->RadiusMax)){
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
	// data.. x -> ode
	for (Model *o: world.objects){
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
	// data.. ode -> x
	for (Object *o: world.objects){
		if (o){
			dBodyID b = (dBodyID)o->body_id;
			if (b != 0){
				o->pos = *(vector*)dBodyGetPosition(b);
				o->vel = *(vector*)dBodyGetLinearVel(b);
				o->rot = *(vector*)dBodyGetAngularVel(b);
				qode2x(dBodyGetQuaternion(b), &o->ang);
				o->update_matrix();
				o->update_theta();
				o->_ResetPhysAbsolute_();
			}
		}
	}
}

#endif

void GodDoCollisionDetection()
{
#ifdef _X_ALLOW_X_
#ifdef _X_ALLOW_PHYSICS_DEBUG_
	PhysicsDebugColData.Num = 0;
#endif
	
	// object <-> terrain
	for (int i=0;i<world.objects.num;i++)
		if (world.objects[i])
			if (world.objects[i]->physics_data.active)
				if (!world.objects[i]->frozen)
					Test4Ground(world.objects[i]);

	TestObjectSanity("God::Coll   1");

	// object <-> object
	for (int i=0;i<world.objects.num;i++)
		if (world.objects[i])
			for (int j=i+1;j<world.objects.num;j++)
				if (world.objects[j])
					if ((!world.objects[i]->frozen) || (!world.objects[j]->frozen))
						if (!ObjectsLinked(world.objects[i], world.objects[j]))
							Test4Object(world.objects[i],world.objects[j]);

	TestObjectSanity("God::Coll   2");
	
	DoCollisionObservers();

#ifdef _X_ALLOW_PHYSICS_DEBUG_
	PhysicsTimeCol += HuiGetTime(PhysicsTimer);
#endif
#endif
}

void ApplyGravity()
{
	for (Object *o: world.objects)
		if (o)
			if (!o->frozen){
				float ttf = o->time_till_freeze;
				vector g = o->physics_data.mass * o->physics_data.g_factor * world.gravity;
				o->add_force(g, v_0);
				//                                                     GetG(o->Pos));
				o->time_till_freeze = ttf;
			}
}

void ResetExternalForces()
{
	for (Object *o: world.objects)
		if (o)
			o->force_ext = o->torque_ext = v_0;
}


void GodCalcMove(float dt)
{
	if (dt == 0)
		return;

	//CreateObjectLists();
	engine.NumRealColTests = 0;

#ifdef _X_ALLOW_PHYSICS_DEBUG_
	HuiGetTime(PhysicsTimer);
#endif

	TestObjectSanity("God::CM prae");
	bool phys_en = engine.PhysicsEnabled;// and (!engine.FirstFrame);
	bool coll_en = engine.CollisionsEnabled;// and (!engine.FirstFrame);

	// no physics?
	if (!phys_en){
		for (int i=0;i<world.objects.num;i++)
			if (world.objects[i])
				world.objects[i]->update_matrix();
		ResetExternalForces();
		return;
	}else if (!coll_en){
		for (int i=0;i<world.objects.num;i++)
			if (world.objects[i])
				//if (!world.objects[i]->Frozen)
					world.objects[i]->do_physics(dt);
		ResetExternalForces();
		return;
	}

	// force fields
	//DoForceFields();

	// add gravitation
	ApplyGravity();
	

	// do the physics several times to increase precision!
	float elapsed_temp = dt;
	engine.Elapsed /= (float)PhysicsNumSteps;
	for (int ttt=0;ttt<PhysicsNumSteps;ttt++){

#ifdef USE_ODE
		PhysicsDataToODE();
#endif

		TestObjectSanity("God::CM prae Links");

#ifdef _X_ALLOW_PHYSICS_DEBUG_
		PhysicsTimePhysics += HuiGetTime(PhysicsTimer);
#endif

		// statics and hinges...
#ifdef _X_ALLOW_X_
		DoLinks(PhysicsNumLinkSteps);
#endif

		TestObjectSanity("God::CM post Links");

#ifndef USE_ODE
		// propagate through space
		for (int i=0;i<world.objects.num;i++)
			if (world.objects[i])
				//if (!world.objects[i]->Frozen)
					world.objects[i]->DoPhysics();
#endif


		TestObjectSanity("God::CM   1");

#ifdef _X_ALLOW_PHYSICS_DEBUG_
		PhysicsTimePhysics += HuiGetTime(PhysicsTimer);
#endif

		// collision detection
		GodDoCollisionDetection();

#ifdef USE_ODE
		// physics...
		dWorldQuickStep(world_id, engine.Elapsed);
		dJointGroupEmpty(contactgroup);
		PhysicsDataFromODE();
#endif

		TestObjectSanity("God::CM   2");


	}
	engine.Elapsed = elapsed_temp;

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

void GodDoAnimation(float dt)
{
	// mainly model animation
	for (int i=0;i<world.objects.num;i++)
		if (world.objects[i])
			world.objects[i]->do_animation(dt);
}

void GodIterateObjects(float dt)
{
	for (Object* o: world.objects)
		if (o)
			o->on_iterate(dt);
}

void Test4Ground(Object *o)
{
#ifdef _X_ALLOW_X_
	for (int i=0;i<world.terrains.num;i++){
		if (CollideObjectTerrain(o, world.terrains[i])){
			//msg->Write(string2("   col %d <-> terrain %d",i,j));

			if (inf_v(o->vel))
				msg_error("inf   Test4Ground Vel 1");

			// script callback function
			o->on_collide_t(world.terrains[i]);

			world.terrain_object->material[0]->friction = world.terrains[i]->material->friction;
			world.terrain_object->object_id = i + 0x40000000; //(1<<30);
			ColData.o2 = world.terrain_object;
			
			// physical reaction
#ifdef _X_ALLOW_PHYSICS_DEBUG_
			if (Elapsed > 0)
#endif
			HandleCollision();

	// debugging
	if (inf_v(o->vel)){
		msg_error("inf   Test4Ground Vel 2");
		msg_write(o->script_data.name);
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

	// Kollision?
	if (!CollideObjects(o1, o2))
		return;

	// debugging
	if (inf_v(o1->vel))
			msg_error("inf   Test4Object Vel 1   (1)");
	if (inf_v(o2->vel))
			msg_error("inf   Test4Object Vel 1   (2)");

	// script callback functions
	o1->on_collide_m(o2);
	o2->on_collide_m(o1);

	// physical reaction
#ifdef _X_ALLOW_PHYSICS_DEBUG_
	if (Elapsed > 0)
#endif	
	HandleCollision();
	

	// debugging
	if (inf_v(o1->vel)){
		msg_error("inf   Test4Object Vel 2    (1)");
		msg_write(o1->script_data.name);
		msg_write(o2->script_data.name);
		msg_write(format("num = %d", ColData.num));
		for (int j=0;j<ColData.num;j++){
			msg_write(format("n = (%f  %f  %f)", ColData.normal[j].x, ColData.normal[j].y, ColData.normal[j].z));
			msg_write(format("p = (%f  %f  %f)", ColData.pos[j].x, ColData.pos[j].y, ColData.pos[j].z));
			msg_write(format("depth = %f", ColData.depth));
		}
	}
	if (inf_v(o2->vel)){
		msg_error("inf   Test4Object Vel 2    (2)");
		msg_write(o1->script_data.name);
		msg_write(o2->script_data.name);
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
	vector dir = p2 - p1;
	float range = dir.length();
	dir /= range;
	float dmin = range;
	data.type = TRACE_TYPE_NONE;
	for (Terrain* t: world.terrains){
		if (t->Trace(p1, p2, dir, dmin, data, simple_test)){
			if (simple_test)
				return true;
			float d = (p1 - data.point).length();
			if (d < dmin){
				dmin = d;
				data.terrain = t;
			}
		}
	}
	for (Object* o: world.objects)
		if (o){
			if (o == o_ignore)
				continue;
			vector p2t = p1 + dir * dmin;
			if (o->Trace(p1, p2t, dir, dmin, data, simple_test)){
				if (simple_test)
					return true;
				float d = (p1 - data.point).length();
				if (d < dmin){
					dmin = d;
					data.object = o;
				}
			}
		}
	return (dmin < range);
}

bool GodTraceVisual(const vector &p1, const vector &p2, TraceData &data, bool simple_test, Model *o_ignore)
{
	vector dir = p2 - p1;
	float range = dir.length();
	dir /= range;
	float dmin = range;
	data.type = TRACE_TYPE_NONE;
	for (Terrain* t: world.terrains){
		if (t->Trace(p1, p2, dir, dmin, data, simple_test)){
			if (simple_test)
				return true;
			float d = (p1 - data.point).length();
			if (d < dmin){
				dmin = d;
				data.terrain = t;
			}
		}
	}
	for (Object* o: world.objects)
		if (o){
			if (o == o_ignore)
				continue;
			vector p2t = p1 + dir * dmin;
			if (o->TraceMesh(p1, p2t, dir, dmin, data, simple_test)){
				if (simple_test)
					return true;
				float d = (p1 - data.point).length();
				if (d < dmin){
					dmin = d;
					data.object = o;
				}
			}
		}
	return (dmin < range);
}

// do everything needed before drawing the objects
void GodPreDraw(vector &cam_pos)
{
	world.add_all_objects_to_lists = false;

// sort models by depth
	/*MetaClearSorted();
	// objects
	for (int i=0;i<NumObjects;i++)
		if (ObjectExisting[i])
			Objects[i]->Draw(cam_pos);*/
}

void GodDrawSorted();

// actually draw the objects
void GodDraw()
{
	// draw the sorted models
	//MetaDrawSorted();
	GodDrawSorted();

	// force fields ....(obsolete?!)
#ifdef _X_ALLOW_X_
	nix::set_alpha(ALPHA_SOURCE_ALPHA, ALPHA_ONE);
	nix::set_cull(CULL_NONE);
	nix::set_z(false,true);
	for (int i=0;i<world.NumForceFields;i++)
		if (world.ForceField[i]->Visible){
			color c=color(world.ForceField[i]->TimeToLife/4,1,1,1);
			nix::set_material(c,c,Black,0,Black);
			//Fx::DrawBall(ForceField[i]->Pos,ForceField[i]->Radius,8,16);
		}
	nix::set_z(true,true);
	nix::set_cull(CULL_DEFAULT);
	nix::set_alpha(nix::AlphaMode::NONE);
#endif
}
#endif

void World::register_object(Model *o, int index) {
	int on = index;
	if (on < 0){
		// ..... better use a list of "empty" objects???
		for (int i=num_reserved_objects; i<objects.num; i++)
			if (!objects[i])
				on = i;
	}else{
		if (on >= objects.num)
			objects.resize(on+1);
		if (objects[on]){
			msg_error("CreateObject:  object index already in use " + i2s(on));
			return;
		}
	}
	if (on < 0){
		on = objects.num;
		objects.add(NULL);
	}
	objects[on] = (Object*)o;

	register_model(o);

	o->object_id = on;

#ifdef USE_ODE
	if (o->physics_data.active){
		o->body_id = dBodyCreate(world_id);
		dBodySetPosition((dBodyID)o->body_id, o->pos.x, o->pos.y, o->pos.z);
		dMass m;
		dMassSetParameters(&m, o->physics_data.mass, 0, 0, 0, o->physics_data.theta._00, o->physics_data.theta._11, o->physics_data.theta._22, o->physics_data.theta._01, o->physics_data.theta._02, o->physics_data.theta._12);
		dBodySetMass((dBodyID)o->body_id, &m);
		dBodySetData((dBodyID)o->body_id, o);
		dQuaternion qq;
		qx2ode(&o->ang, qq);
		dBodySetQuaternion((dBodyID)o->body_id, qq);
	}else
		o->body_id = 0;

	vector d = o->prop.max - o->prop.min;
	o->geom_id = dCreateBox(space_id, d.x, d.y, d.z); //dCreateSphere(0, 1); // space, radius
//	msg_write((int)o->geom_id);
	dGeomSetBody((dGeomID)o->geom_id, (dBodyID)o->body_id);
#endif
}



// un-object a model
void World::unregister_object(Model *m) {
	if (m->object_id < 0)
		return;

#ifdef USE_ODE
	if (m->body_id != 0)
		dBodyDestroy((dBodyID)m->body_id);
	m->body_id = 0;

	if (m->geom_id != 0)
		dGeomDestroy((dGeomID)m->geom_id);
	m->geom_id = 0;
#endif

	// ego...
	if (m == ego)
		ego = NULL;

	AddNetMsg(NET_MSG_DELETE_OBJECT, m->object_id, "");

	// remove from list
	objects[m->object_id] = NULL;
	m->object_id = -1;
}

void PartialModel::clear() {
//	delete ubo;
//	delete dset;
}

// add a model to the (possible) rendering list
void World::register_model(Model *m) {
	if (m->registered)
		return;
	
	for (int i=0;i<m->material.num;i++){
		Material *mat = m->material[i];
		bool trans = false;//!mat->alpha.z_buffer; //false;
		/*if (mat->TransparencyMode>0){
			if (mat->TransparencyMode == TransparencyMode::FUNCTIONS)
				trans = true;
			if (mat->TransparencyMode == TransparencyMode::FACTOR)
				trans = true;
		}*/

		PartialModel p;
		p.model = m;
		p.material = mat;
//		p.ubo = new vulkan::UniformBuffer(64*3);
//		p.dset = rp_create_dset(mat->textures, p.ubo);
		p.mat_index = i;
		p.transparent = trans;
		p.shadow = false;
		if (trans)
			sorted_trans.add(p);
		else
			sorted_opaque.add(p);
	}

#ifdef _X_ALLOW_FX_
	for (int i=0;i<m->fx.num;i++)
		if (m->fx[i])
			m->fx[i]->enable(true);
#endif
	
	m->registered = true;
	
	// sub models
	for (int i=0;i<m->bone.num;i++)
		if (m->bone[i].model)
			register_model(m->bone[i].model);
}

// remove a model from the (possible) rendering list
void World::unregister_model(Model *m) {
	if (!m->registered)
		return;
	//printf("%p   %s\n", m, MetaGetModelFilename(m));

	foreachi (auto &s, sorted_trans, i)
		if (s.model == m) {
			s.clear();
			sorted_trans.erase(i);
		}
	foreachi (auto &s, sorted_opaque, i)
		if (s.model == m) {
			s.clear();
			sorted_opaque.erase(i);
		}

#ifdef _X_ALLOW_FX_
	if (!engine.resetting_game)
		for (int i=0;i<m->fx.num;i++)
			if (m->fx[i])
				m->fx[i]->enable(false);
#endif
	
	m->registered = false;
	//printf("%d\n", m->NumBones);

	// sub models
	for (int i=0;i<m->bone.num;i++)
		if (m->bone[i].model)
			unregister_model(m->bone[i].model);
}

void World::add_light(Light *l) {
	lights.add(l);
}

void World::add_particle(Particle *p) {
	//particle_manager->add(p);
}

#if 0
void WorldShiftAll(const vector &dpos)
{
	for (Object *o: world.objects)
		if (o)
			o->pos += dpos;
	for (Terrain *t: world.terrains)
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
		float dist_2 = dist * engine.DetailFactorInv; // more adequate distance value

		// ignore?
		if (is_ignored(m))
			continue;

		// which level of detail?
		if (dist_2 > m->prop.detail_dist[2]){		detail = -1;	continue;	}
		else if (dist_2 > m->prop.detail_dist[1])	detail = SKIN_LOW;
		else if (dist_2 > m->prop.detail_dist[0])	detail = SKIN_MEDIUM;

		PartialModelView pmv;
		pmv.p = &p[i];
		pmv.z = dist;
		pmv.detail = detail;
		vp.add(pmv);

		// shadows...
#ifdef _X_ALLOW_X_
		if ((engine.ShadowLevel > 0) and (detail == SKIN_HIGH) and (p[i].mat_index == 0) and (m->prop.allow_shadow)){
			int sd = engine.ShadowLowerDetail ? SKIN_MEDIUM : SKIN_HIGH;
			if (m->skin[sd]->sub[p[i].mat_index].num_triangles > 0)
				Fx::AddShadow(m, sd);
		}
#endif
	}
	if ((do_sort) and (vp.num > 0)){
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
	vector dir = cur_cam->ang * vector::EZ;
	quaternion a2;
	a2 = cur_cam->ang * quaternion::rotation_v(vector(0, +0.9f, 0));
	vector dir_l = a2 * vector::EZ;
	a2 = cur_cam->ang * quaternion::rotation_v(vector(0, -0.9f, 0));
	vector dir_r = a2 * vector::EZ;
	a2 = cur_cam->ang * quaternion::rotation_v(vector(+1.0f, 0, 0));
	vector dir_t = a2 * vector::EZ;
	a2 = cur_cam->ang * quaternion::rotation_v(vector(-1.0f, 0, 0));
	vector dir_b = a2 * vector::EZ;
	
	for (int i=0;i<vp.num;i++){
		PartialModel *p = (PartialModel*)vp[i].p;
		Model *m = p->model;
		vector dpos = pos - m->pos;

		// camera frustrum testing
		if (dpos * dir > m->prop.radius)
			continue;
		if (dpos * dir_l > m->prop.radius)
			continue;
		if (dpos * dir_r > m->prop.radius)
			continue;
		if (dpos * dir_t > m->prop.radius)
			continue;
		if (dpos * dir_b > m->prop.radius)
			continue;

#ifdef _X_ALLOW_X_
		Light::Apply(m->pos);
#endif

		// draw!
		//p->material->apply();
		m->material[p->mat_index]->apply();
		//m->draw(0, m->_matrix, true, false);//p->shadow);
		m->draw_simple(p->mat_index, vp[i].detail);
	}
}

static int fill_frame = 100;

int ffframe = 0;

void GroupDuplicates()
{
	for (int i=0;i<world.objects.num;i++){}
}

void GodDrawSorted()
{
	ffframe ++;
	if ((ffframe % 100) == 0)
		GroupDuplicates();

// fill data structures
	//fill_frame ++;
	//if (fill_frame > 10){
		// TODO: refill when SortedNonTrans or SortedNonTrans change... pointers...vector allocation...
		vector pos = cur_cam->pos;
		fill_pmv(pos, SortedNonTrans, cur_cam->pmvd.opaque, engine.SortingEnabled, true);
		fill_pmv(pos, SortedTrans, cur_cam->pmvd.trans, true, false);
		fill_frame = 0;
	//}

// non-transparent models
	// overlapping each other
	if (engine.ZBufferEnabled)
		nix::set_z(true,true);
	else
		nix::set_z(false,false);

	// drawing
	draw_pmv(cur_cam->pmvd.opaque);

//transparent models
	// test but don't write
	if (engine.ZBufferEnabled)
		nix::set_z(false,true);

	// drawing
	draw_pmv(cur_cam->pmvd.trans);

	
	// reset the z buffer
	nix::set_z(true,true);
}

#endif
