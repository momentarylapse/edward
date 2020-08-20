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
|                                                                              |
| vital properties:                                                            |
|  - vertex buffers get filled temporaryly per frame                           |
|                                                                              |
| last update: 2008.10.26 (c) by MichiSoft TM                                  |
\*----------------------------------------------------------------------------*/
#include "model.h"
#include "Material.h"
#include "world.h"
#include "../meta.h"
#if HAS_LIB_VULKAN
#include "../lib/vulkan/vulkan.h"
#else
#include "../lib/nix/nix.h"
#endif
#include "../lib/file/file.h"


#define DynamicNormalCorrect


//#define MODEL_MAX_EDGES			65536

float col_frac(const color &a, const color &b);

void MoveTimeAdd(Model *m,int operation_no,float elapsed,float v,bool loop);

bool Model::AllowDeleteRecursive = true;


ModelTemplate::ModelTemplate(Model *m) {
	script = NULL;
	model = m;
}

MetaMove::MetaMove() {
	num_frames_skeleton = 0;
	num_frames_vertex = 0;
	this->skel_ang = NULL;
	this->skel_dpos = NULL;
	this->mesh[0].dpos = NULL;
	this->mesh[1].dpos = NULL;
	this->mesh[2].dpos = NULL;
	this->mesh[3].dpos = NULL;
}


// make a copy of all the data
#if 0
Mesh* Mesh::copy(Model *new_owner) {
	Mesh *s = new Mesh;
	*s = *this;

	// subs
	for (auto &ss: s->sub){
		// reset the vertex buffers
		ss.vertex_buffer = NULL;
		ss.force_update = true;
	}
	s->owner = new_owner;
	return s;
}

// update physical data in world coordinates
void Model::_UpdatePhysAbsolute_()
{
	PhysicalMeshAbsolute *a = &phys_absolute;
	PhysicalMesh *s = phys;


	/*bool nn=true;
	for (int i=0;i<16;i++){
		msg_write(f2s(m.e[i],3));
		nn &= ( m.e[i] == 0 );
	}
	if (nn){
		HuiRaiseError("null-matrix...kille");
	}*/


	if (!a->is_ok){
		a->p.resize(s->vertex.num);
		a->pl.resize(s->poly.num * MODEL_MAX_POLY_FACES);
		// transform vertices
		for (int i=0;i<s->vertex.num;i++)
			//VecTransform(a->p[i],Matrix,s->Vertex[i]);
			a->p[i] = _matrix * s->vertex[i];
		// convex polyhedron
		for (int i=0;i<s->poly.num;i++)
			for (int k=0;k<s->poly[i].num_faces;k++)
				a->pl[i * MODEL_MAX_POLY_FACES+k] = s->poly[i].face[k].pl.transform(_matrix);
		a->is_ok = true;
	}
	
	// doesn't need to be recursive... already done by GetCollision()!
	/*for (int i=0;i<Bone.num;i++)
		if (BoneModel[i]){
			MatrixMultiply(BoneMatrix[i],Matrix,BoneDMatrix[i]);
			BoneModel[i]->_UpdatePhysAbsolute_(BoneMatrix[i]);
		}*/
}
#endif

// mark data as obsolete
void Model::_ResetPhysAbsolute_()
{
	phys_absolute.is_ok=false;
	for (int i=0;i<bone.num;i++)
		if (bone[i].model)
			bone[i].model->_ResetPhysAbsolute_();
}

//--------------------------------------------------------------------------------------------------
// hopefully these functions will be obsolete with the next fileformat

// how big is the model
void AppraiseDimensions(Model *m)
{
	float rad = 0;
	
	// bounding box (visual skin[0])
	m->prop.min = m->prop.max = v_0;
	for (int i=0;i<m->mesh[0]->vertex.num;i++){
		m->prop.min._min(m->mesh[0]->vertex[i]);
		m->prop.max._max(m->mesh[0]->vertex[i]);
		float r = _vec_length_fuzzy_(m->mesh[0]->vertex[i]);
		if (r > rad)
			rad = r;
	}

	// physical skin
	for (int i=0;i<m->phys->vertex.num;i++){
		float r = _vec_length_fuzzy_(m->phys->vertex[i]);
		if (r > rad)
			rad = r;
	}
	for (auto &b: m->phys->balls){
		float r = _vec_length_fuzzy_(m->phys->vertex[b.index]) + b.radius;
		if (r > rad)
			rad = r;
	}
	m->prop.radius = rad;
}



void create_vb(Model *m, Mesh *mesh) {
	for (auto &s: mesh->sub) {
		s.vertex_buffer = new nix::VertexBuffer("3f,3f,2f");
		Array<vector> p, n;
		Array<float> uv;
		for (int i=0; i<s.num_triangles; i++) {
			for (int k=0; k<3; k++) {
				int vi = s.triangle_index[i*3+k];
				p.add(mesh->vertex[vi]);
				n.add(s.normal[i*3+k]);
				uv.add(s.skin_vertex[i*6+k*2  ]);
				uv.add(s.skin_vertex[i*6+k*2+1]);
			}
		}
		s.vertex_buffer->update(0, p);
		s.vertex_buffer->update(1, n);
		s.vertex_buffer->update(2, uv);


#if LIB_HAS_VULKAN
		s.vertex_buffer = new vulkan::VertexBuffer();
		Array<vulkan::Vertex1> vertices;
		for (int i=0; i<s.num_triangles; i++) {
			vulkan::Vertex1 v;
			for (int k=0; k<3; k++) {
				int vi = s.triangle_index[i*3+k];
				v.pos = mesh->vertex[vi];
				v.normal = s.normal[i*3+k];
				v.u = s.skin_vertex[i*6+k*2  ];
				v.v = s.skin_vertex[i*6+k*2+1];
				vertices.add(v);
			}
		}
		s.vertex_buffer->build1(vertices);
#endif
	}
}

#if 0

// make sure we have enough vertex buffers
void CreateVB(Model *m, Mesh *s)
{
	for (int t=0;t<s->sub.num;t++){
		if (!s->sub[t].vertex_buffer)
			s->sub[t].vertex_buffer = new vulkan::VertexBuffer(m->material[t]->textures.num);
		s->sub[t].force_update = true;
	}
}

#endif

void PostProcessSkin(Model *m, Mesh *s) {

	create_vb(m,s);

	// bounding box
	s->min = s->max = v_0;
	if (s->vertex.num > 0){
		s->min = s->max = s->vertex[0];
		for (int i=0;i<s->vertex.num;i++){
			s->min._min(s->vertex[i]);
			s->max._max(s->vertex[i]);
		}
	}
}

void PostProcessPhys(Model *m, PhysicalMesh *s)
{
	m->phys_absolute.p.clear();
	m->phys_absolute.pl.clear();
	m->_ResetPhysAbsolute_();
}

color file_read_color4i(File *f)
{
	int a = f->read_int();
	int r = f->read_int();
	int g = f->read_int();
	int b = f->read_int();
	return color((float)a/255.0f, (float)r/255.0f, (float)g/255.0f, (float)b/255.0f);
}


static vector get_normal_by_index(int index)
{
	float wz = (float)(index >> 8) * pi / 255.0f;
	float wxy = (float)(index & 255) * 2 * pi / 255.0f;
	float swz = sin(wz);
	if (swz < 0)
		swz = - swz;
	float cwz = cos(wz);
	return vector( cos(wxy) * swz, sin(wxy) * swz, cwz);
}

void Model::reset_data()
{
	registered = false;
	object_id = -1;
	parent = NULL;
	
	for (int i=0;i<MODEL_NUM_MESHES;i++){
		_detail_needed_[i] = false;
		vertex_dyn[i].clear();
		normal_dyn[i].clear();
	}

	// "auto-animate"
	anim.num_operations = -1;
	anim.operation[0].move = 0;
	anim.operation[0].time = 0;
	anim.operation[0].operation = MOVE_OP_SET;
	anim.operation[0].param1 = 0;
	anim.operation[0].param2 = 0;
	if (anim.meta){
		for (int i=0;i<anim.meta->move.num;i++)
			if (anim.meta->move[i].num_frames > 0){
				anim.operation[0].move = i;
				break;
			}
	}

	if (physics_data.active)
		physics_data.mass_inv = 1.0f / physics_data.mass;
	else
		physics_data.mass_inv = 0;
	physics_data.theta = physics_data.theta_0;
	physics_data.g_factor = 1;
	physics_data.test_collisions = true;
	prop.allow_shadow = false;
	
	vel = rot = v_0;
}

void read_color(File *f, color &c)
{
	c.a = (float)f->read_int() / 255.0f;
	c.r = (float)f->read_int() / 255.0f;
	c.g = (float)f->read_int() / 255.0f;
	c.b = (float)f->read_int() / 255.0f;
}

// completely load an original model (all data is its own)
void Model::load(const Path &filename)
{
	msg_right();

	// load model from file
	File *f = FileOpenText(filename);
	int ffv = f->ReadFileFormatVersion();
	if (ffv != 11){
		FileClose(f);
		msg_left();
		throw Exception(format("wrong file format: %d (11 expected)", ffv));
	}

	Array<float> temp_sv;
	_template = new ModelTemplate(this);
	_template->filename = filename;

// file format 11...
	// General
	f->read_comment();
	// bounding box
	f->read_vector(&prop.min);
	f->read_vector(&prop.max);
	// skins
	f->read_int();
	// reserved
	f->read_int();
	f->read_int();
	f->read_int();

	// Materials
	f->read_comment();
	int num_materials = f->read_int();
	material.resize(num_materials);
	for (int i=0;i<material.num;i++){
		Material *m = LoadMaterial(f->read_str());
		material[i] = m;
		bool user_colors = f->read_bool();
		if (user_colors){
			color am, sp;
			am = file_read_color4i(f);
			m->diffuse = file_read_color4i(f);
			sp = file_read_color4i(f);
			m->emission = file_read_color4i(f);
			m->shininess = (float)f->read_int();
			m->ambient = col_frac(am, m->diffuse) / 2;
			m->specular = col_frac(am, White);
		}else{
			file_read_color4i(f);
			file_read_color4i(f);
			file_read_color4i(f);
			file_read_color4i(f);
			f->read_int();
		}
		int alpha_mode = f->read_int();
		if (alpha_mode != TRANSPARENCY_DEFAULT){
			m->alpha.mode = alpha_mode;
			m->alpha.source = f->read_int();
			m->alpha.destination = f->read_int();
			m->alpha.factor = (float)f->read_int() * 0.01f;
			m->alpha.z_buffer = f->read_bool();
		}else{
			f->read_int();
			f->read_int();
			f->read_int();
			f->read_bool();

		}
		int nt = f->read_int();
		if (nt > m->textures.num)
			m->textures.resize(nt);
		for (int t=0;t<nt;t++){
			string fn = f->read_str();
			m->textures[t] = nix::LoadTexture(fn);
		}
	}
	
	// Physical Skin
	phys = new PhysicalMesh;
	phys_is_reference = false;
	//   vertices
	f->read_comment();
	int nv = f->read_int();
	phys->bone_nr.resize(nv);;
	phys->vertex.resize(nv);
	for (int i=0;i<nv;i++)
		phys->bone_nr[i] = f->read_int();
	for (int i=0;i<nv;i++)
		f->read_vector(&phys->vertex[i]);
	//   triangles
	f->read_int();
	//   balls
	int nb = f->read_int();
	phys->balls.resize(nb);
	for (int i=0;i<nb;i++){
		phys->balls[i].index = f->read_int();
		phys->balls[i].radius = f->read_float();
	}
	//   convex polyhedron
	int np = f->read_int();
	phys->poly.resize(np);
	for (int i=0;i<np;i++){
		ConvexPolyhedron *p = &phys->poly[i];
		p->num_faces = f->read_int();
		for (int j=0;j<p->num_faces;j++){
			p->face[j].num_vertices = f->read_int();
			for (int k=0;k<p->face[j].num_vertices;k++)
				p->face[j].index[k] = f->read_int();
			p->face[j].pl.n.x = f->read_float();
			p->face[j].pl.n.y = f->read_float();
			p->face[j].pl.n.z = f->read_float();
			p->face[j].pl.d = f->read_float();
		}
		// non redundand stuff
		p->num_vertices = f->read_int();
		p->vertex = new int[p->num_vertices];
		for (int k=0;k<p->num_vertices;k++)
			p->vertex[k] = f->read_int();
		p->num_edges = f->read_int();
		p->edge_index = new int[p->num_edges * 2];
		for (int k=0;k<p->num_edges*2;k++)
			p->edge_index[k] = f->read_int();
		// topology
		p->faces_joining_edge = new int[p->num_faces * p->num_faces];
		for (int k=0;k<p->num_faces;k++)
			for (int l=0;l<p->num_faces;l++)
				p->faces_joining_edge[k * p->num_faces + l] = f->read_int();
		p->edge_on_face = new bool[p->num_edges * p->num_faces];
		for (int k=0;k<p->num_edges;k++)
			for (int l=0;l<p->num_faces;l++)
			    p->edge_on_face[k * p->num_faces + l] = f->read_bool();
	}

	// Visible Skin[d]
	for (int d=0;d<3;d++){
		mesh[d] = new Mesh;
		Mesh *s = mesh[d];
		s->owner = this;
		s->sub.resize(material.num);
		mesh_is_reference[d] = false;

		// vertices
		f->read_comment();
		int n_vert = f->read_int();
		s->vertex.resize(n_vert);
		s->bone_index.resize(n_vert);
		for (int i=0;i<s->vertex.num;i++)
			f->read_vector(&s->vertex[i]);
		for (int i=0;i<s->vertex.num;i++)
			s->bone_index[i] = f->read_int();

		// skin vertices
		int NumSkinVertices = f->read_int();
		temp_sv.resize(NumSkinVertices * 2);
		for (int i=0;i<NumSkinVertices * 2;i++)
			temp_sv[i] = f->read_float();

		// sub skins
		for (int m=0;m<material.num;m++){
			auto *sub = &s->sub[m];
			// triangles
			sub->num_triangles = f->read_int();
			sub->triangle_index.resize(sub->num_triangles * 3);
			sub->skin_vertex.resize(material[m]->textures.num * sub->num_triangles * 6);
			sub->normal.resize(sub->num_triangles * 3);
			// vertices
			for (int i=0;i<sub->num_triangles * 3;i++)
				sub->triangle_index[i] = f->read_int();
			// skin vertices
			for (int i=0;i<material[m]->textures.num * sub->num_triangles * 3;i++){
				int sv = f->read_int();
				sub->skin_vertex[i * 2    ] = temp_sv[sv * 2    ];
				sub->skin_vertex[i * 2 + 1] = temp_sv[sv * 2 + 1];
			}
			// normals
			for (int i=0;i<sub->num_triangles * 3;i++)
				sub->normal[i] = get_normal_by_index(f->read_int());

			f->read_int();
			sub->force_update = true;
			sub->vertex_buffer = NULL;
		}
		f->read_int();
	}

	// Skeleton
	f->read_comment();
	bone.resize(f->read_int());
	for (int i=0;i<bone.num;i++){
		f->read_vector(&bone[i].pos);
		bone[i].parent = f->read_int();
		_template->bone_model_filename.add(f->read_str());
		bone[i].model = NULL; //LoadModel(...);
	}

	// Animations
	f->read_comment();
	int num_anims_all = f->read_int();
	int num_anims = f->read_int();
	int num_frames_vert = f->read_int();
	int num_frames_skel = f->read_int();
	// animated?
	anim.meta = NULL;
	if (num_anims_all > 0){
		anim.meta = new MetaMove;
		anim.meta->num_frames_skeleton = num_frames_skel;
		anim.meta->num_frames_vertex = num_frames_vert;

		anim.meta->move.resize(num_anims_all);
		//memset(anim.meta->move, 0, sizeof(Move) * num_anims_all);
		if (num_frames_vert > 0)
			for (int i=0;i<4;i++){
				int n_vert = 0;
				if (phys)
					n_vert = phys->vertex.num;
				if (i > 0)
					n_vert = mesh[i - 1]->vertex.num;
				anim.meta->mesh[i].dpos = new vector[num_frames_vert * n_vert];
				memset(anim.meta->mesh[i].dpos, 0, sizeof(vector) * num_frames_vert * n_vert);
			}
		if (num_frames_skel > 0){
			anim.meta->skel_dpos = new vector[num_frames_skel * bone.num];
			anim.meta->skel_ang = new quaternion[num_frames_skel * bone.num];
		}
		int frame_s = 0, frame_v = 0;

		// moves
		for (int i=0;i<num_anims;i++){
			int index = f->read_int();
			
			// auto animation: use first move!
			if (i==0)
				anim.operation[0].move = index;
			Move *m = &anim.meta->move[index];
			f->read_str(); // name is irrelevant
			m->type = f->read_int();
			m->num_frames = f->read_int();
			m->frames_per_sec_const = f->read_float();
			m->frames_per_sec_factor = f->read_float();
			
			if (m->type == MOVE_TYPE_VERTEX){
				m->frame0 = frame_v;
				for (int fr=0;fr<m->num_frames;fr++){
					for (int s=0;s<4;s++){
						// s=-1: Phys
						int np = phys->vertex.num;
						if (s >= 1)
							np = mesh[s - 1]->vertex.num;
						int num_vertices = f->read_int();
						for (int j=0;j<num_vertices;j++){
							int vertex_index = f->read_int();
							f->read_vector(&anim.meta->mesh[s].dpos[frame_v * np + vertex_index]);
						}
					}
					frame_v ++;
				}
			}else if (m->type == MOVE_TYPE_SKELETAL){
				m->frame0 = frame_s;
				bool *free_pos = new bool[bone.num];
				for (int j=0;j<bone.num;j++)
					free_pos[j] = f->read_bool();
				m->inter_quad = f->read_bool();
				m->inter_loop = f->read_bool();
				for (int fr=0;fr<m->num_frames;fr++){
					for (int j=0;j<bone.num;j++){
						vector v;
						f->read_vector(&v);
						anim.meta->skel_ang[frame_s * bone.num + j] = quaternion::rotation_v( v);
						if (free_pos[j])
							f->read_vector(&anim.meta->skel_dpos[frame_s * bone.num + j]);
					}
					frame_s ++;
				}
				delete[](free_pos);
			}
		}
	}
	temp_sv.clear();

	// Effects
	f->read_comment();
	int num_fx = f->read_int();
	_template->fx.resize(num_fx);
	for (int i=0;i<num_fx;i++){
		ModelEffectData *d = &_template->fx[i];
		string fxtype = f->read_str();
		if (fxtype == "Script"){
			d->type = FX_TYPE_SCRIPT;
			d->vertex = f->read_int();
			d->filename = f->read_str();
			f->read_str();
		}else if (fxtype == "Light"){
			d->type = FX_TYPE_LIGHT;
			d->vertex = f->read_int();
			d->radius = f->read_float();
			read_color(f, d->am);
			read_color(f, d->di);
			read_color(f, d->sp);
		}else if (fxtype == "Sound"){
			d->type=FX_TYPE_SOUND;
			d->vertex=f->read_int();
			d->radius = (float)f->read_int();
			d->speed = (float)f->read_int()*0.01f;
			d->filename = f->read_str();
		}else if (fxtype == "ForceField"){
			d->type =FX_TYPE_FORCEFIELD;
			f->read_int();
			f->read_int();
			f->read_int();
			f->read_bool();
		}else
			msg_error("unknown effect: " + fxtype);
	}

	// Physics
	f->read_comment();
	physics_data.mass = f->read_float();
	for (int i=0;i<9;i++)
		physics_data.theta_0.e[i] = f->read_float();
	physics_data.active = f->read_bool();
	physics_data.passive = f->read_bool();
	prop.radius = f->read_float();

	// LOD-Distances
	f->read_comment();
	prop.detail_dist[MESH_HIGH] = f->read_float();
	prop.detail_dist[MESH_MEDIUM] = f->read_float();
	prop.detail_dist[MESH_LOW] = f->read_float();
	

// object data
	// Object Data
	f->read_comment();
	script_data.name = f->read_str();
	script_data.description = f->read_str();

	// Inventary
	f->read_comment();
	script_data.inventary.resize(f->read_int());
	for (int i=0;i<script_data.inventary.num;i++){
		_template->inventory_filename.add(f->read_str());
		//script_data.inventary[i] = LoadModel(f->read_str());
		f->read_int();
	}

	// Script
	f->read_comment();
	_template->script_filename = f->read_str();
	script_data.var.resize(f->read_int());
	for (int i=0;i<script_data.var.num;i++)
		script_data.var[i] = f->read_float();

	while (true){
		string s = f->read_str();
		if (s == "// Cylinders"){
			int nc = f->read_int();
			phys->cylinders.resize(nc);
			for (int i=0; i<nc; i++){
				phys->cylinders[i].index[0] = f->read_int();
				phys->cylinders[i].index[1] = f->read_int();
				phys->cylinders[i].radius = f->read_float();
				phys->cylinders[i].round = f->read_bool();
			}
		}else if (s == "// Script Vars"){
			int nv = f->read_int();
			for (int i=0; i<nv; i++){
				TemplateDataScriptVariable v;
				v.name = f->read_str().lower().replace("_", "");
				v.value = f->read_str();
				_template->variables.add(v);
			}
		}else{
			break;
		}
	}

	FileClose(f);



	// do some post processing...
	AppraiseDimensions(this);

	for (int i=0;i<MODEL_NUM_MESHES;i++)
		PostProcessSkin(this, mesh[i]);

	PostProcessPhys(this, phys);




	// skeleton
	if (bone.num>0){
		bone_pos_0.resize(bone.num);
		for (int i=0;i<bone.num;i++){
			bone_pos_0[i] = _GetBonePos(i);
			bone[i].dmatrix = matrix::translation( bone_pos_0[i]);
		}
	}
	


	is_copy = false;
	reset_data();

	msg_left();
}


Model::Model() {
	pos = vel = rot = v_0;
	ang = quaternion::ID;
	object_id = -1;
	registered = false;
	on_ground = false;
	visible = true;
	rotating  = true;
	moved = false;
	frozen = false;
	time_till_freeze = 0;
	ground_id = -1;
	ground_normal = v_0;
	_detail_ = -1;

	is_copy = false;
	_template = NULL;

	for (int i=0;i<MODEL_NUM_MESHES;i++){
		mesh_is_reference[i] = false;
		_detail_needed_[i] = false;
		mesh[i] = NULL;
	}
	anim.meta = NULL;
	phys = NULL;

	vel_surf = acc = v_0;
	force_int = torque_int = v_0;
	force_ext = torque_ext = v_0;

	body = nullptr;
	colShape = nullptr;


	parent = nullptr;
	phys_is_reference = false;

	anim.num_operations = 0;
}

void Model::__init__()
{
	new(this) Model;
}

#if 0
void CopyPhysicalSkin(PhysicalMesh *orig, PhysicalMesh **copy)
{
	(*copy) = new PhysicalMesh;
	(**copy) = (*orig);
}
#endif

Model *Model::copy(Model *pre_allocated)
{
	if (is_copy)
		msg_error("model: copy of copy");

	Model *m = pre_allocated;
	if (!m)
		m = new Model();

	m->prop = prop;
	m->physics_data = physics_data;
	m->_template = _template;
	m->script_data = script_data;

	for (Material* mat: material)
		m->material.add(mat->copy());

	m->anim.meta = anim.meta;

	m->fx.clear();
	

	// "copy" presettings (just using references)
	m->is_copy = true;
	for (int i=0;i<MODEL_NUM_MESHES;i++){
		m->mesh[i] = mesh[i];
		m->mesh_is_reference[i] = true;
		m->_detail_needed_[i] = false;
	}
	m->phys = phys;
	m->phys_is_reference = true;
	m->registered = false;
	m->visible = true;

#if 0
	// skins
	if ((anim.meta) or (bone.num > 0)){
		for (int i=0;i<MODEL_NUM_MESHES;i++){
			// mostly needs it's own vertex_buffer...but well
			m->mesh[i] = mesh[i]->copy(m);
			m->mesh_is_reference[i] = false;
		}
	}
#endif

	// skeleton
	m->bone = bone;
	if (bone.num > 0){
		m->bone_pos_0 = bone_pos_0;

		for (int i=0;i<bone.num;i++)
			if (bone[i].model)
				m->bone[i].model = NULL;//CopyModel(bone[i].model, allow_script_init);
	}

	// effects
	// loaded by ResetData()

	m->_ResetPhysAbsolute_();



	// reset
	m->reset_data();

	return m;
}

void ExternalModelCleanup(Model *m);

// delete only the data owned by this model
//    don't delete sub models ...done by meta
Model::~Model()
{
	ExternalModelCleanup(this);

	if (AllowDeleteRecursive){
		// delete sub models
		for (Bone &b: bone)
			if (b.model)
				delete(b.model);

		// delete inventary
		for (Model *i: script_data.inventary)
			if (i)
				delete(i);
	}

	// animation
	if ((anim.meta) and (!is_copy)){
		if (anim.meta->skel_dpos)
			delete[](anim.meta->skel_dpos);
		if (anim.meta->skel_ang)
			delete[](anim.meta->skel_ang);
		for (int i=0;i<4;i++)
			if (anim.meta->mesh[i].dpos)
				delete[](anim.meta->mesh[i].dpos);
		delete(anim.meta);
	}

	// physical
	if (phys and !phys_is_reference){
		for (auto &p: phys->poly){
			delete[](p.vertex);
			delete[](p.edge_index);
			delete[](p.edge_on_face);
			delete[](p.faces_joining_edge);
		}
		delete(phys);
	}

	// skin
	for (int i=0;i<MODEL_NUM_MESHES;i++)
		if (mesh[i] and !mesh_is_reference[i]){
			Mesh *s = mesh[i];
			if (s->owner != this)
				continue;

			// vertex buffer
			for (int t=0;t<s->sub.num;t++)
				if (s->sub[t].vertex_buffer)
					delete(s->sub[t].vertex_buffer);

			// own / own data
			delete(mesh[i]);
		}

	for (Material* m: material)
		delete(m);

	// template
	if (!is_copy)
		delete(_template);
}

void Model::__delete__()
{
	this->Model::~Model();
}

// non-animated state
vector Model::_GetBonePos(int index)
{
	int r = bone[index].parent;
	if (r < 0)
		return bone[index].pos;
	return bone[index].pos + _GetBonePos(r);
}

#if 0
int get_num_trias(Skin *s)
{
	int n = 0;
	for (int i=0;i<s->sub.num;i++)
		n += s->sub[i].num_triangles;
	return n;
}

void Model::SetBoneModel(int index, Model *sub)
{
	if ((index < 0) or (index >= bone.num)){
		msg_error(format("Model::SetBoneModel: (%s) invalid bone index %d", filename().c_str(), index));
		return;
	}
	
	// remove the old one
	if (bone[index].model){
		bone[index].model->parent = NULL;
		if (registered)
			GodUnregisterModel(bone[index].model);
	}

	// add the new one
	bone[index].model = sub;
	if (sub){
		sub->parent = this;
		if (registered)
			GodRegisterModel(sub);
	}
}

Model *Model::root()
{
	Model *next = this;
	while(next->parent)
		next = next->parent;
	return next;
}

void Model::do_animation(float elapsed)
{
	if (anim.meta){

		// for handling special cases (-1,-2)
		int num_ops = anim.num_operations;

		// "auto-animated"
		if (anim.num_operations == -1){
			// default: just run a single animation
			MoveTimeAdd(this, 0, elapsed, 0, true);
			anim.operation[0].operation = MOVE_OP_SET;
			anim.operation[0].move = 0;
			num_ops = 1;
		}

		// skeleton edited by script...
		if (anim.num_operations == -2){
			num_ops = 0;
		}

		// make sure we have something to store the animated data in...
		for (int s=0;s<MODEL_NUM_MESHES;s++)
			if (_detail_needed_[s]){
				vertex_dyn[s].resize(skin[s]->vertex.num);
				//memset(vertex_dyn[s], 0, sizeof(vector) * Skin[s]->vertex.num);
				memcpy(&vertex_dyn[s][0], &skin[s]->vertex[0], sizeof(vector) * skin[s]->vertex.num);
				int nt = get_num_trias(skin[s]);
				normal_dyn[s].resize(nt * 3);
				int offset = 0;
				for (int i=0;i<material.num;i++){
					memcpy(&normal_dyn[s][offset], &skin[s]->sub[i].normal[0], sizeof(vector) * skin[s]->sub[i].num_triangles * 3);
					offset += skin[s]->sub[i].num_triangles;
				}
			}


	// vertex animation

		bool vertex_animated=false;
		for (int op=0;op<num_ops;op++){
			if (anim.operation[op].move<0)	continue;
			Move *m = &anim.meta->move[anim.operation[op].move];
			//msg_write(GetFilename() + format(" %d %d %p %d", op, anim.operation[op].move, m, m->num_frames));
			if (m->num_frames == 0)
				continue;


			if (m->type == MOVE_TYPE_VERTEX){
				vertex_animated=true;

				// frame data
				int fr=(int)(anim.operation[op].time); // current frame (relative)
				float dt=anim.operation[op].time-(float)fr;
				int f1 = m->frame0 + fr; // current frame (absolute)
				int f2 = m->frame0 + (fr+1)%m->num_frames; // next frame (absolute)

				// transform vertices
				for (int s=0;s<MODEL_NUM_MESHES;s++)
					if (_detail_needed_[s]){
						Skin *sk = skin[s];
						for (int p=0;p<sk->vertex.num;p++){
							vector dp1 = anim.meta->skin[s + 1].dpos[f1 * sk->vertex.num + p]; // first value
							vector dp2 = anim.meta->skin[s + 1].dpos[f2 * sk->vertex.num + p]; // second value
							vertex_dyn[s][p] += dp1*(1-dt) + dp2*dt;
						}
						for (int i=0;i<sk->sub.num;i++)
							sk->sub[i].force_update = true;
					}
			}
		}
		
	// skeletal animation

		for (int i=0;i<bone.num;i++){
			Bone *b = &bone[i];

			// reset (only if not being edited by script)
			/*if (Numanim.operations != -2){
				b->cur_ang = quaternion::ID;//quaternion(1, v_0);
				b->cur_pos = b->Pos;
			}*/

			// operations
			for (int iop=0;iop<num_ops;iop++){
				MoveOperation *op = &anim.operation[iop];
				if (op->move < 0)
					continue;
				Move *m = &anim.meta->move[op->move];
				if (m->num_frames == 0)
					continue;
				if (m->type != MOVE_TYPE_SKELETAL)
					continue;
				quaternion w,w0,w1,w2,w3;
				vector p,p1,p2;

			// calculate the alignment belonging to this argument
				int fr = (int)(op->time); // current frame (relative)
				int f1 = m->frame0 + fr; // current frame (absolute)
				int f2 = m->frame0 + (fr+1)%m->num_frames; // next frame (absolute)
				float df = op->time-(float)fr; // time since start of current frame
				w1 = anim.meta->skel_ang[f1 * bone.num + i]; // first value
				p1 = anim.meta->skel_dpos[f1*bone.num + i];
				w2 = anim.meta->skel_ang[f2*bone.num + i]; // second value
				p2 = anim.meta->skel_dpos[f2*bone.num + i];
				m->inter_quad = false;
				/*if (m->InterQuad){
					w0=m->ang[i][(f-1+m->NumFrames)%m->NumFrames]; // last value
					w3=m->ang[i][(f+2             )%m->NumFrames]; // third value
					// interpolate the current alignment
					w = quaternion::interpolate(w0,w1,w2,w3,df);
					p=(1.0f-df)*p1+df*p2 + SkeletonDPos[i];
				}else*/{
					// interpolate the current alignment
					w = quaternion::interpolate(w1,w2,df);
					p=(1.0f-df)*p1+df*p2 + b->pos;
				}


			// execute the operations

				// overwrite
				if (op->operation == MOVE_OP_SET){
					b->cur_ang = w;
					b->cur_pos = p;

				// overwrite, if current doesn't equal 0
				}else if (op->operation == MOVE_OP_SET_NEW_KEYED){
					if (w.w!=1)
						b->cur_ang=w;
					if (p!=v_0)
						b->cur_pos=p;

				// overwrite, if last equals 0
				}else if (op->operation == MOVE_OP_SET_OLD_KEYED){
					if (b->cur_ang.w==1)
						b->cur_ang=w;
					if (b->cur_pos==v_0)
						b->cur_pos=p;

				// w = w_old         + w_new * f
				}else if (op->operation == MOVE_OP_ADD_1_FACTOR){
					w = w.scale_angle(op->param1);
					b->cur_ang = w * b->cur_ang;
					b->cur_pos += op->param1 * p;

				// w = w_old * (1-f) + w_new * f
				}else if (op->operation == MOVE_OP_MIX_1_FACTOR){
					b->cur_ang = quaternion::interpolate( b->cur_ang, w, op->param1);
					b->cur_pos = (1 - op->param1) * b->cur_pos + op->param1 * p;

				// w = w_old * a     + w_new * b
				}else if (op->operation == MOVE_OP_MIX_2_FACTOR){
					b->cur_ang = b->cur_ang.scale_angle(op->param1);
					w = w.scale_angle(op->param2);
					b->cur_ang = quaternion::interpolate( b->cur_ang, w, 0.5f);
					b->cur_pos = op->param1 * b->cur_pos + op->param2 * p;
				}
			}

			// bone has root -> align to root
			if (b->parent >= 0)
				b->cur_pos = bone[b->parent].dmatrix * b->pos;

			// create matrices (model -> skeleton)
			matrix t,r;
			t = matrix::translation( b->cur_pos);
			r = matrix::rotation_q( b->cur_ang);
			MatrixMultiply(b->dmatrix, t, r);
		}

		// create the animated data
		if (bone.num > 0)
			for (int s=0;s<MODEL_NUM_MESHES;s++){
				if (!_detail_needed_[s])
					continue;
				Skin *sk = skin[s];
				// transform vertices
				for (int p=0;p<sk->vertex.num;p++){
					int b = sk->bone_index[p];
					vector pp = sk->vertex[p] - bone_pos_0[b];
					// interpolate vertex
					vertex_dyn[s][p] = bone[b].dmatrix * pp;
					//vertex_dyn[s][p]=pp;
				}
				// normal vectors
				int offset = 0;
				for (int mm=0;mm<sk->sub.num;mm++){
					SubSkin *sub = &sk->sub[mm];
				#ifdef DynamicNormalCorrect
					for (int t=0;t<sub->num_triangles*3;t++)
						normal_dyn[s][t + offset] = bone[sk->bone_index[sub->triangle_index[t]]].dmatrix.transform_normal(sub->normal[t]);
						//normal_dyn[s][t + offset]=sub->Normal[t];
				#else
					memcpy(&normal_dyn[s][offset], &sub->Normal[0], sub->num_triangles * 3 * sizeof(vector));
				#endif
					sub->force_update = true;
					offset += sub->num_triangles;
				}
			}
	}


	
	// reset for the next frame
	for (int s=0;s<MODEL_NUM_MESHES;s++)
		_detail_needed_[s] = false;

	// update effects
	/*for (int i=0;i<NumFx;i++){
		sEffect **pfx=(sEffect**)fx;
		int nn=0;
		while( (*pfx) ){
			vector vp;
			VecTransform(vp, Matrix, Skin[0]->vertex[(*pfx)->vertex]);
			FxUpdateByModel(*pfx,vp,vp);
			pfx++;
		}
	}*/

	// recursion
	for (int i=0;i<bone.num;i++)
		if (bone[i].model){
			MatrixMultiply(bone[i].model->_matrix, _matrix, bone[i].dmatrix);
			bone[i].model->do_animation(elapsed);
		}
}

float line_dist(const vector &pa, const vector &da, const vector &pb, const vector &db, vector &pointa, vector &pointb)
{
	plane pl;
	vector n = da ^ db; // shortest direction
	n.normalize();
	pl = plane::from_point_normal( pb, n ^ db); // line b in pl
	pl.intersect_line(pa, pa + da, pointa);
	float d = VecLineDistance(pointa, pb, pb + db);
	vector x1 = pointa - n * d;
	vector x2 = pointa + n * d;
	if (VecLineDistance(x1, pb, pb + db) < VecLineDistance(x2, pb, pb + db))
		pointb = x1;
	else
		pointb = x2;
	return d;
}



//###############################################################################
// kuerzester Abstand:
// spaeter mit _vec_between_ und unterschiedlichen TPs rechnen, statt mit _vec_length_
//###############################################################################
bool Model::Trace(const vector &p1, const vector &p2, const vector &dir, float range, TraceData &data, bool simple_test)
{
	if (!physics_data.passive)
		return false;
	
	bool hit=false;
	vector c;
	float dmin=range;
	// skeleton -> recursion
	if (bone.num>0){
		vector c;
		for (int i=0;i<bone.num;i++)
			if (bone[i].model){
				vector p2t=p1+dmin*p2;
				if (bone[i].model->Trace(p1, p2t, dir, dmin, data, simple_test)){
					if (simple_test)
						return true;
					hit=true;
					float d=_vec_length_(p1-c);
					if (d < dmin){
						dmin=d;
					}
				}
			}
		range=dmin;
	}

	if (!phys)
		return hit;
	if (phys->vertex.num < 1)
		return hit;

	dmin=range+1;
	float d;


// Modell nah genug am Trace-Strahl?
	plane pl;
	vector tm = (p1+p2)/2; // Mittelpunkt des Trace-Strahles
	// Wuerfel um Modell und Trace-Mittelpunkt berschneiden sich?
	if (!pos.bounding_cube(tm, prop.radius + range / 2))
		return false;
	// Strahl schneidet Ebene mit Modell (senkrecht zum Strahl)
	pl = plane::from_point_normal( pos, dir);
	_plane_intersect_line_(c,pl,p1,p2);
	// Schnitt nah genau an Modell?
	if (!pos.bounding_cube(c, prop.radius * 2))
		return false;

	// sich selbst absolut ausrichten
	_UpdatePhysAbsolute_();
	vector *p = &phys_absolute.p[0];

	//msg_write(this->_template->filename);

// Trace-Test
	// Kugeln
	for (auto &b: phys->balls){
		pl = plane::from_point_normal( p[b.index],dir);
		if (!_plane_intersect_line_(c,pl,p1,p2))
			continue;
		if (_vec_length_fuzzy_(p[b.index]-c) > b.radius)
			continue;
		float d = _vec_length_(p[b.index]-c);
		if (d < b.radius){
			c -= dir*(float)(sqrt(b.radius * b.radius - d * d));
			if (!_vec_between_(c, p1, p2))
				continue;
			if (simple_test)
				return true;
			d  =_vec_length_(c-p1);
			// naehester Tracepunkt?
			if (d < dmin){
				hit = true;
				dmin = d;
				data.point = c;
				data.model = this;
				data.type = TRACE_TYPE_MODEL;
			}
		}
	}

	for (auto &c: phys->cylinders){
		vector &ca = p[c.index[0]];
		vector &cb = p[c.index[1]];
		vector cd = cb - ca;
		cd.normalize();
		vector cp, tp;
		float d = line_dist(p1, dir, ca, cd, tp, cp);
		if (d > c.radius)
			continue;
		//tp -= dir*(float)(sqrt(c.radius * c.radius - d * d)); // FIXME...
		if (!_vec_between_(tp, p1, p2))
			continue;
		if (!_vec_between_(cp, ca, cb))
			continue;
		d = 0;//(tp - p1).length();
		if (d < dmin){
			dmin = d;
			hit = true;
			data.point = tp;
			data.model = this;
			data.type = TRACE_TYPE_MODEL;
		}
	}

	// Polyeder
	foreachi (auto &pp, phys->poly, i){
		for (int k=0;k<pp.num_faces;k++){
			if (!_plane_intersect_line_(c,phys_absolute.pl[i*MODEL_MAX_POLY_FACES+k],p1,p2))
				continue;
			if (!_vec_between_(c,p1,p2))
				continue;
			bool inside=true;
			for (int j=0;j<pp.num_faces;j++)
				if ((j!=k)and(_plane_distance_(phys_absolute.pl[i*MODEL_MAX_POLY_FACES+j],c)>0))
					inside=false;
			if (!inside)
				continue;
			d=_vec_length_(c-p1);
			// naehester Tracepunkt?
			if (d<dmin){
				hit=true;
				dmin=d;
				data.point = c;
				data.model = this;
				data.type = TRACE_TYPE_MODEL;
			}
		}
	}
	//return (dmin<range);
	return hit;
}


bool Model::TraceMesh(const vector &p1, const vector &p2, const vector &dir, float range, TraceData &data, bool simple_test)
{
	return false;
}


vector _cdecl Model::GetVertex(int index, int skin_no)
{
	Skin *s = skin[skin_no];
	vector v;
	if (anim.meta){ // animated
		int b = s->bone_index[index];
		v = s->vertex[index] - bone_pos_0[b];//Move(b);
		v = bone[b].dmatrix * v;
		v = _matrix * v;
	}else{ // static
		v = _matrix * s->vertex[index];
	}
	return v;
}

// reset all animation data for a model (needed in each frame before applying animations!)
void Model::ResetAnimation()
{
	anim.num_operations = 0;
	/*for (int i=0;i<NumSkelettonPoints;i++)
		if (sub_model[i])
			ModelMoveReset(sub_model[i]);*/
}

// did the animation reach its end?
bool Model::IsAnimationDone(int operation_no)
{
	int move_no = anim.operation[operation_no].move;
	if (move_no < 0)
		return true;
	// in case animation doesn't exist
	if (anim.meta->move[move_no].num_frames == 0)
		return true;
	return (anim.operation[operation_no].time >= (float)(anim.meta->move[move_no].num_frames - 1));
}

// dumbly add the correct animation time, ignore animation's ending
void MoveTimeAdd(Model *m,int operation_no,float elapsed,float v,bool loop)
{
	int move_no = m->anim.operation[operation_no].move;
	if (move_no < 0)
		return;
	Move *move = &m->anim.meta->move[move_no];
	if (move->num_frames == 0)
		return; // in case animation doesn't exist

	// how many frames have passed
	float dt = elapsed * ( move->frames_per_sec_const + move->frames_per_sec_factor * v );

	// add the correct time
	m->anim.operation[operation_no].time += dt;
	// time may now be way out of range of the animation!!!

	if (m->IsAnimationDone(operation_no)){
		if (loop)
			m->anim.operation[operation_no].time -= float(move->num_frames) * (int)((float)(m->anim.operation[operation_no].time / move->num_frames));
		else
			m->anim.operation[operation_no].time = (float)(move->num_frames) - 1;
	}
}

// apply an animate to a model
//   a new animation "layer" is being added for mixing several animations
//   the variable <time> is being increased
bool Model::Animate(int mode, float param1, float param2, int move_no, float &time, float elapsed, float vel_param, bool loop)
{
	if (!anim.meta)
		return false;
	if (anim.num_operations < 0){
		anim.num_operations = 0;
	}else if (anim.num_operations >= MODEL_MAX_MOVE_OPS - 1){
		msg_error("Model.Animate(): no more than " + i2s(MODEL_MAX_MOVE_OPS) + " animation layers allowed");
		return false;
	}
	int n = anim.num_operations ++;
	anim.operation[n].move = move_no;
	anim.operation[n].operation = mode;
	anim.operation[n].param1 = param1;
	anim.operation[n].param2 = param2;
	anim.operation[n].time = time;

	MoveTimeAdd(this, n, elapsed, vel_param, loop);
	time = anim.operation[n].time;
	return IsAnimationDone(n);
}

// get the number of frames for a particular animation
int Model::GetFrames(int move_no)
{
	if (!anim.meta)
		return 0;
	return anim.meta->move[move_no].num_frames;
}

// edit skelettal animation via script
void Model::BeginEditAnimation()
{
	if (!anim.meta)
		return;
	anim.num_operations = -2;
	for (int i=0;i<bone.num;i++){
		bone[i].cur_ang = quaternion::ID;
		bone[i].cur_pos = bone[i].pos;
	}
}

// make sure, the model/skin is editable and not just a reference.... (soon)
void Model::BeginEdit(int detail)
{
	make_editable();
	if (skin_is_reference[detail]){
		skin[detail] = skin[detail]->copy(this);
		skin_is_reference[detail] = false;
	}
}

// force an update for this model/skin
void Model::EndEdit(int detail)
{
	for (int i=0;i<material.num;i++)
		skin[detail]->sub[i].force_update = true;
}


// make sure we can edit this object without destroying an original one
void Model::make_editable()
{
}

string Model::filename()
{
	if (_template)
		return _template->filename;
	return "?";
}
#endif


void Model::draw_simple(int mat_no, int detail) {
	_detail_needed_[detail] = true;
	auto *s = mesh[detail];
	auto &sub = s->sub[mat_no];

	//-----------------------------------------------------

	if (sub.force_update)
		update_vertex_buffer(mat_no, detail);


	nix::SetWorldMatrix(_matrix);
	nix::DrawTriangles(sub.vertex_buffer);

/*	if ((m->cube_map >= 0) and (m->reflection_density > 0)){
		//_Pos_ = *_matrix_get_translation_(_matrix);
		NixSetMaterial(m->ambient, Black, Black, 0, m->emission);
		FxCubeMapDraw(m->cube_map, sub.vertex_buffer, m->reflection_density);
	}*/
}

#if 0
void Model::SortingTest(vector &pos,const vector &dpos,matrix *mat,bool allow_shadow)
{
	for (int i=0;i<bone.num;i++)
		if (boneModel[i]){
			vector sub_pos;
			MatrixMultiply(boneMatrix[i],*mat,boneDMatrix[i]);
			VecTransform(sub_pos,boneMatrix[i],v_0);
			boneModel[i]->SortingTest(sub_pos,dpos,&BoneMatrix[i],allow_shadow);
		}
	int _Detail_=SKIN_HIGH;
	bool trans=false;
	float ld=_vec_length_(dpos); // real distance to the camera
	float ld_2=ld*DetailFactorInv; // more adequate distance value

	// which level of detail?
	if (ld_2>DetailDist[0])		_Detail_=SKIN_MEDIUM;
	if (ld_2>DetailDist[1])		_Detail_=SKIN_LOW;
	if (ld_2>DetailDist[2]){	_Detail_=-1;	return;	}

	// transparent?
	if (Material[0].TransparencyMode>0){
		if (Material[0].TransparencyMode==TransparencyModeFunctions)
			trans=true;
		if (Material[0].TransparencyMode==TransparencyModeFactor)
			trans=true;
	}

	if (anim.meta){
		_Detail_Needed_[_Detail_]=true;
		if ( ( ShadowLevel > 0) and ShadowLowerDetail and allow_shadow and ( _Detail_ == SKIN_HIGH ) )
			_Detail_Needed_[SKIN_MEDIUM]=true; // shadows...
	}

	// send for sorting
	MetaAddSorted(this,pos,mat,_Detail_,ld,trans,allow_shadow);
}

void Model::update_vertex_buffer(int mat_no, int detail)
{
	Skin *s = skin[detail];
	SubSkin &sub = s->sub[mat_no];
	Material *m = material[mat_no];

	vector *p = &s->vertex[0];
	if (vertex_dyn[detail].num > 0)
		p = &vertex_dyn[detail][0];
	vector *n = &sub.normal[0];
	if (normal_dyn[detail].num)
		n = &normal_dyn[detail][0];
	int *tv = &sub.triangle_index[0];
	float *sv = &sub.skin_vertex[0];

	// vertex buffer existing?
	if (!sub.vertex_buffer){
		sub.vertex_buffer = new nix::VertexBuffer(m->textures.num);
	}
	sub.vertex_buffer->clear();
	if (m->textures.num == 1){
		for (int i=0;i<sub.num_triangles;i++){
			//msg_write(i);
			int va=tv[i*3  ]  ,vb=tv[i*3+1]  ,vc=tv[i*3+2];
			sub.vertex_buffer->addTria(
							p[va],n[i*3  ],sv[i*6  ],sv[i*6+1],
							p[vb],n[i*3+1],sv[i*6+2],sv[i*6+3],
							p[vc],n[i*3+2],sv[i*6+4],sv[i*6+5]);
		}
	}else{
		for (int i=0;i<sub.num_triangles;i++){
			float tc[3][MATERIAL_MAX_TEXTURES * 2];
			for (int k=0;k<3;k++)
				for (int j=0;j<m->textures.num;j++){
					tc[k][j * 2    ] = sub.skin_vertex[j * sub.num_triangles * 6 + i * 6 + k * 2];
					tc[k][j * 2 + 1] = sub.skin_vertex[j * sub.num_triangles * 6 + i * 6 + k * 2 + 1];
				}
			//msg_write(i);
			int va=tv[i*3  ]  ,vb=tv[i*3+1]  ,vc=tv[i*3+2];
			sub.vertex_buffer->addTriaM(
							p[va],n[i*3  ],tc[0],
							p[vb],n[i*3+1],tc[1],
							p[vc],n[i*3+2],tc[2]);
		}
	}
	sub.force_update = false;

}
#endif

void Model::update_vertex_buffer(int mat_no, int detail) {}

void Model::draw(int detail, bool set_fx, bool allow_shadow)
{
	if (detail < MESH_HIGH)
		return;
//	msg_write("d");
	// shadows?
/*	if ((ShadowLevel>0)and(set_fx)and(detail==SkinHigh)and(allow_shadow)){//(TransparencyMode<=0))
		int sd = ShadowLowerDetail ? SkinMedium : SkinHigh;
		//if (Skin[sd]->num_triangles>0)
			FxAddShadow(this, sd);
			//FxAddShadow(Skin[sd],vertex_dyn[sd],Matrix,100000);//Diameter*5);
	}*/

	// mirror?
	/*if (Material[0].ReflectionMode==ReflectionMirror)
		FxDrawMirrors(Skin[detail],mat);*/


	for (int i=0;i<material.num;i++){
		//material[i]->apply();
		nix::SetTextures(material[i]->textures);
		nix::SetShader(material[i]->shader);

		// finally... really draw!!!
		draw_simple(i, detail);
		//NixSetShader(-1);
		nix::SetAlpha(ALPHA_NONE);
	}
}

