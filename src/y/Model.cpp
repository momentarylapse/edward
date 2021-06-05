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

#include "Model.h"
#include "Material.h"
#include "World.h"
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



File *load_file_x(const Path &filename, int &version) {

	File *f = FileOpen(filename);
	char c = f->read_char();
	if (c == 'b') {
		version = f->read_word();
		return f;
	} else if (c == 't') {
		delete f;
		f = FileOpenText(filename);
		f->read_char();
		version = f->read_word();
		return f;
	} else {
		throw Exception("File format unreadable!");
	}
	return nullptr;
}


ModelTemplate::ModelTemplate(Model *m) {
	script = NULL;
	model = m;
}

MetaMove::MetaMove() {
	num_frames_skeleton = 0;
	num_frames_vertex = 0;
}


SubMesh::SubMesh() {
	vertex_buffer = nullptr;
	force_update = true;
	num_triangles = 0;
}

// make a copy of all the data
Mesh* Mesh::copy(Model *new_owner) {
	Mesh *s = new Mesh;
	*s = *this;

	// subs
	for (auto &ss: s->sub) {
		// reset the vertex buffers
		ss.vertex_buffer = nullptr;
		//ss.force_update = true;
	}
	s->create_vb();
	s->update_vb();

	s->owner = new_owner;
	return s;
}

#if 0
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
void AppraiseDimensions(Model *m) {
	float rad = 0;
	
	// bounding box (visual mesh[0])
	m->prop.min = m->prop.max = v_0;
	for (int i=0;i<m->mesh[0]->vertex.num;i++) {
		m->prop.min._min(m->mesh[0]->vertex[i]);
		m->prop.max._max(m->mesh[0]->vertex[i]);
		float r = _vec_length_fuzzy_(m->mesh[0]->vertex[i]);
		if (r > rad)
			rad = r;
	}

	// physical skin
	for (int i=0;i<m->phys->vertex.num;i++) {
		float r = _vec_length_fuzzy_(m->phys->vertex[i]);
		if (r > rad)
			rad = r;
	}
	for (auto &b: m->phys->balls) {
		float r = _vec_length_fuzzy_(m->phys->vertex[b.index]) + b.radius;
		if (r > rad)
			rad = r;
	}
	m->prop.radius = rad;
}



void SubMesh::create_vb() {
	vertex_buffer = new nix::VertexBuffer("3f,3f,2f");

#if HAS_LIB_VULKAN
	vertex_buffer = new vulkan::VertexBuffer();
#endif
}
void Mesh::create_vb() {
	for (auto &s: sub)
		s.create_vb();
}

void SubMesh::update_vb(Mesh *mesh) {
	Array<vector> p, n;
	Array<float> uv;
	for (int i=0; i<num_triangles; i++) {
		for (int k=0; k<3; k++) {
			int vi = triangle_index[i*3+k];
			p.add(mesh->vertex[vi]);
			n.add(normal[i*3+k]);
			uv.add(skin_vertex[i*6+k*2  ]);
			uv.add(skin_vertex[i*6+k*2+1]);
		}
	}
	vertex_buffer->update(0, p);
	vertex_buffer->update(1, n);
	vertex_buffer->update(2, uv);


#if HAS_LIB_VULKAN
	Array<vulkan::Vertex1> vertices;
	for (int i=0; i<num_triangles; i++) {
		vulkan::Vertex1 v;
		for (int k=0; k<3; k++) {
			int vi = triangle_index[i*3+k];
			v.delta_pos = mesh->vertex[vi];
			v.normal = normal[i*3+k];
			v.u = skin_vertex[i*6+k*2  ];
			v.v = skin_vertex[i*6+k*2+1];
			vertices.add(v);
		}
	}
	vertex_buffer->build1(vertices);
#endif
}

void Mesh::update_vb() {
	for (auto &s: sub)
		s.update_vb(this);
}

void Mesh::post_process() {

	create_vb();
	update_vb();

	// bounding box
	min = max = v_0;
	if (vertex.num > 0) {
		min = max = vertex[0];
		for (int i=0; i<vertex.num; i++) {
			min._min(vertex[i]);
			max._max(vertex[i]);
		}
	}
}

void PostProcessPhys(Model *m, PhysicalMesh *s) {
	m->phys_absolute.p.clear();
	m->phys_absolute.pl.clear();
	m->_ResetPhysAbsolute_();
}

color file_read_color4i(File *f) {
	int a = f->read_int();
	int r = f->read_int();
	int g = f->read_int();
	int b = f->read_int();
	return color((float)a/255.0f, (float)r/255.0f, (float)g/255.0f, (float)b/255.0f);
}


vector get_normal_by_index(int index) {
	float wz = (float)(index >> 8) * pi / 255.0f;
	float wxy = (float)(index & 255) * 2 * pi / 255.0f;
	float swz = sin(wz);
	if (swz < 0)
		swz = - swz;
	float cwz = cos(wz);
	return vector( cos(wxy) * swz, sin(wxy) * swz, cwz);
}

void Model::reset_data() {
	registered = false;
	object_id = -1;
	parent = NULL;
	
	for (int i=0; i<MODEL_NUM_MESHES; i++) {
		_detail_needed_[i] = false;
	}

	// "auto-animate"
	anim.num_operations = -1;
	anim.operation[0].move = 0;
	anim.operation[0].time = 0;
	anim.operation[0].command = MoveOperation::Command::SET;
	anim.operation[0].param1 = 0;
	anim.operation[0].param2 = 0;
	if (anim.meta) {
		for (int i=0; i<anim.meta->move.num; i++)
			if (anim.meta->move[i].num_frames > 0) {
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


Model::Model() : Entity(Type::MODEL) {
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

	is_copy = false;
	_template = NULL;

	for (int i=0;i<MODEL_NUM_MESHES;i++) {
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

void Model::__init__() {
	new(this) Model;
}

#if 0
void CopyPhysicalSkin(PhysicalMesh *orig, PhysicalMesh **copy)
{
	(*copy) = new PhysicalMesh;
	(**copy) = (*orig);
}
#endif

Model *Model::copy(Model *pre_allocated) {
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
	for (int i=0; i<MODEL_NUM_MESHES; i++) {
		m->mesh[i] = mesh[i];
		m->_detail_needed_[i] = false;
	}
	m->phys = phys;
	m->phys_is_reference = true;
	m->registered = false;
	m->visible = true;

#if 1
	// skins
	if (anim.meta or (bone.num > 0)) {
		for (int i=0; i<MODEL_NUM_MESHES; i++) {
			// mostly needs it's own vertex_buffer...but well
			m->anim.mesh[i] = mesh[i]->copy(m);
		}
	}
#endif

	// skeleton
	m->bone = bone;
	for (int i=0;i<bone.num;i++)
		if (bone[i].model)
			m->bone[i].model = NULL;//CopyModel(bone[i].model, allow_script_init);

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
Model::~Model() {
	ExternalModelCleanup(this);

	if (AllowDeleteRecursive) {
		// delete sub models
		for (Bone &b: bone)
			if (b.model)
				delete b.model;

		// delete inventary
		for (Model *i: script_data.inventary)
			if (i)
				delete i;
	}

	// animation
	if (anim.meta and !is_copy)
		delete anim.meta;

	// physical
	if (phys and !phys_is_reference)
		delete phys;

	// skin
	for (int i=0;i<MODEL_NUM_MESHES;i++)
		if (mesh[i]) {
			Mesh *s = mesh[i];
			if (s->owner != this)
				continue;

			// vertex buffer
			for (int t=0;t<s->sub.num;t++)
				if (s->sub[t].vertex_buffer)
					delete s->sub[t].vertex_buffer;

			// own / own data
			delete mesh[i];
		}

	for (Material* m: material)
		delete m;

	// template
	if (!is_copy)
		delete _template;
}

void Model::__delete__() {
	this->Model::~Model();
}

// non-animated state
vector Model::get_bone_rest_pos(int index) const {
	auto &b = bone[index];
	if (b.parent >= 0)
		return b.delta_pos + get_bone_rest_pos(b.parent);
	return b.delta_pos;
}

int get_num_trias(Mesh *s) {
	int n = 0;
	for (int i=0;i<s->sub.num;i++)
		n += s->sub[i].num_triangles;
	return n;
}

/*void Model::set_bone_model(int index, Model *sub) {
	auto &b = bone[index];
	
	// remove the old one
	if (b.model) {
		if (registered)
			GodUnregisterModel(b.model);
		b.model->parent = nullptr;
	}

	// add the new one
	b.model = sub;
	if (sub) {
		sub->parent = this;
		if (registered)
			GodRegisterModel(sub);
	}
}*/

Model *Model::root() {
	Model *next = this;
	while (next->parent)
		next = next->parent;
	return next;
}

void Model::do_animation(float elapsed) {

	// recursion
	for (auto &b: bone)
		if (b.model) {
			b.model->_matrix = _matrix * b.dmatrix;
			b.model->do_animation(elapsed);
		}

	if (!anim.meta)
		return;

	// FIXME
	for (int s=0;s<MODEL_NUM_MESHES;s++)
		_detail_needed_[s] = true;


	// for handling special cases (-1,-2)
	int num_ops = anim.num_operations;

	// "auto-animated"
	if (anim.num_operations == -1){
		// default: just run a single animation
		MoveTimeAdd(this, 0, elapsed, 0, true);
		num_ops = 1;
	}

	// skeleton edited by script...
	if (anim.num_operations == -2){
		num_ops = 0;
	}

	// make sure we have something to store the animated data in...
#if 0
	for (int s=0;s<MODEL_NUM_MESHES;s++)
		if (_detail_needed_[s]) {
///			anim.vertex[s].resize(mesh[s]->vertex.num);
			//memset(anim.vertex[s], 0, sizeof(vector) * Skin[s]->vertex.num);
///			memcpy(&anim.vertex[s][0], &mesh[s]->vertex[0], sizeof(vector) * mesh[s]->vertex.num);
			int nt = get_num_trias(mesh[s]);
//			anim.normal[s].resize(nt * 3);
			int offset = 0;
			for (int i=0;i<material.num;i++){
				memcpy(&anim.normal[s][offset], &mesh[s]->sub[i].normal[0], sizeof(vector) * mesh[s]->sub[i].num_triangles * 3);
				offset += mesh[s]->sub[i].num_triangles;
			}
		}
#endif


// vertex animation

	bool vertex_animated = false;
	for (int op=0;op<num_ops;op++){
		if (anim.operation[op].move < 0)
			continue;
		Move *m = &anim.meta->move[anim.operation[op].move];
		//msg_write(GetFilename() + format(" %d %d %p %d", op, anim.operation[op].move, m, m->num_frames));
		if (m->num_frames == 0)
			continue;


		if (m->type == AnimationType::VERTEX){
			vertex_animated=true;

			// frame data
			int fr=(int)(anim.operation[op].time); // current frame (relative)
			float dt=anim.operation[op].time-(float)fr;
			int f1 = m->frame0 + fr; // current frame (absolute)
			int f2 = m->frame0 + (fr+1)%m->num_frames; // next frame (absolute)

			// transform vertices
			for (int s=0;s<MODEL_NUM_MESHES;s++)
				if (_detail_needed_[s]){
					auto *sk = mesh[s];
					auto *a = anim.mesh[s];
					for (int p=0;p<sk->vertex.num;p++){
						vector dp1 = anim.meta->mesh[s + 1].dpos[f1 * sk->vertex.num + p]; // first value
						vector dp2 = anim.meta->mesh[s + 1].dpos[f2 * sk->vertex.num + p]; // second value
						a->vertex[p] = sk->vertex[p] + dp1*(1-dt) + dp2*dt;
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
			if (m->type != AnimationType::SKELETAL)
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
				p=(1.0f-df)*p1+df*p2 + b->delta_pos;
			}


		// execute the operations

			// overwrite
			if (op->command == op->Command::SET){
				b->cur_ang = w;
				b->cur_pos = p;

			// overwrite, if current doesn't equal 0
			}else if (op->command == op->Command::SET_NEW_KEYED){
				if (w.w!=1)
					b->cur_ang=w;
				if (p!=v_0)
					b->cur_pos=p;

			// overwrite, if last equals 0
			}else if (op->command == op->Command::SET_OLD_KEYED){
				if (b->cur_ang.w==1)
					b->cur_ang=w;
				if (b->cur_pos==v_0)
					b->cur_pos=p;

			// w = w_old         + w_new * f
			}else if (op->command == op->Command::ADD_1_FACTOR){
				w = w.scale_angle(op->param1);
				b->cur_ang = w * b->cur_ang;
				b->cur_pos += op->param1 * p;

			// w = w_old * (1-f) + w_new * f
			}else if (op->command == op->Command::MIX_1_FACTOR){
				b->cur_ang = quaternion::interpolate( b->cur_ang, w, op->param1);
				b->cur_pos = (1 - op->param1) * b->cur_pos + op->param1 * p;

			// w = w_old * a     + w_new * b
			}else if (op->command == op->Command::MIX_2_FACTOR){
				b->cur_ang = b->cur_ang.scale_angle(op->param1);
				w = w.scale_angle(op->param2);
				b->cur_ang = quaternion::interpolate( b->cur_ang, w, 0.5f);
				b->cur_pos = op->param1 * b->cur_pos + op->param2 * p;
			}
		}

		// bone has root -> align to root
		if (b->parent >= 0)
			b->cur_pos = bone[b->parent].dmatrix * b->delta_pos;

		// create matrices (model -> skeleton)
		auto t = matrix::translation(b->cur_pos);
		auto r = matrix::rotation_q(b->cur_ang);
		b->dmatrix = t * r;
	}

	// create the animated data
	if (bone.num > 0)
		for (int s=0;s<MODEL_NUM_MESHES;s++){
			if (!_detail_needed_[s])
				continue;
			auto sk = mesh[s];
			// transform vertices
			for (int p=0;p<sk->vertex.num;p++){
				int b = sk->bone_index[p];
				vector pp = sk->vertex[p] - bone[b].rest_pos;
				// interpolate vertex
				anim.mesh[s]->vertex[p] = bone[b].dmatrix * pp;
				//anim.vertex[s][p]=pp;
			}
			// normal vectors
			int offset = 0;
			for (int mm=0;mm<sk->sub.num;mm++){
				auto sub = &sk->sub[mm];
			#ifdef DynamicNormalCorrect
				for (int t=0;t<sub->num_triangles*3;t++)
					anim.mesh[s]->sub[mm].normal[t] = bone[sk->bone_index[sub->triangle_index[t]]].dmatrix.transform_normal(sub->normal[t]);
					//anim.normal[s][t + offset] = ...
					//anim.normal[s][t + offset]=sub->Normal[t];
			#else
				memcpy(&anim.normal[s][offset], &sub->Normal[0], sub->num_triangles * 3 * sizeof(vector));
			#endif
				sub->force_update = true;
				offset += sub->num_triangles;
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

}


#if 0

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
	if (!delta_pos.bounding_cube(tm, prop.radius + range / 2))
		return false;
	// Strahl schneidet Ebene mit Modell (senkrecht zum Strahl)
	pl = plane::from_point_normal( delta_pos, dir);
	_plane_intersect_line_(c,pl,p1,p2);
	// Schnitt nah genau an Modell?
	if (!delta_pos.bounding_cube(c, prop.radius * 2))
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
#endif


vector _cdecl Model::get_vertex(int index) {
	auto s = mesh[MESH_HIGH];
	vector v;
	if (anim.meta) { // animated
		int b = s->bone_index[index];
		v = s->vertex[index] - bone[b].rest_pos;
		v = bone[b].dmatrix * v;
		v = _matrix * v;
	} else { // static
		v = _matrix * s->vertex[index];
	}
	return v;
}

// reset all animation data for a model (needed in each frame before applying animations!)
void Model::reset_animation() {
	anim.num_operations = 0;
}

// did the animation reach its end?
bool Model::is_animation_done(int operation_no) {
	int move_no = anim.operation[operation_no].move;
	if (move_no < 0)
		return true;
	// in case animation doesn't exist
	if (anim.meta->move[move_no].num_frames == 0)
		return true;
	return (anim.operation[operation_no].time >= (float)(anim.meta->move[move_no].num_frames - 1));
}


// dumbly add the correct animation time, ignore animation's ending
void MoveTimeAdd(Model *m, int operation_no, float elapsed, float v, bool loop) {
	auto &op = m->anim.operation[operation_no];
	int move_no = op.move;
	if (move_no < 0)
		return;
	Move *move = &m->anim.meta->move[move_no];
	if (move->num_frames == 0)
		return; // in case animation doesn't exist

	// how many frames have passed
	float dt = elapsed * ( move->frames_per_sec_const + move->frames_per_sec_factor * v );

	// add the correct time
	op.time += dt;
	// time may now be way out of range of the animation!!!

	if (m->is_animation_done(operation_no)) {
		if (loop)
			op.time -= float(move->num_frames) * (int)((float)(op.time / move->num_frames));
		else
			op.time = (float)(move->num_frames) - 1;
	}
}

// apply an animate to a model
//   a new animation "layer" is being added for mixing several animations
//   the variable <time> is being increased
bool Model::animate_x(MoveOperation::Command cmd, float param1, float param2, int move_no, float &time, float dt, float vel_param, bool loop) {
	if (!anim.meta)
		return false;
	if (anim.num_operations < 0){
		anim.num_operations = 0;
	}else if (anim.num_operations >= MODEL_MAX_MOVE_OPS - 1){
		msg_error("Model.Animate(): no more than " + i2s(MODEL_MAX_MOVE_OPS) + " animation layers allowed");
		return false;
	}
	int index = -1;
	foreachi (auto &m, anim.meta->move, i)
		if (m.id == move_no)
			index = i;
	if (index < 0) {
		msg_error("move id not existing: " + i2s(move_no));
		return false;
	}
	int n = anim.num_operations ++;
	anim.operation[n].move = index;
	anim.operation[n].command = cmd;
	anim.operation[n].param1 = param1;
	anim.operation[n].param2 = param2;
	anim.operation[n].time = time;

	MoveTimeAdd(this, n, dt, vel_param, loop);
	time = anim.operation[n].time;
	return is_animation_done(n);
}

bool Model::animate(MoveOperation::Command cmd, int move_no, float &time, float dt, bool loop) {
	return animate_x(cmd, 0, 0, move_no, time, dt, 0, loop);

}

// get the number of frames for a particular animation
int Model::get_frames(int move_no) {
	if (!anim.meta)
		return 0;
	return anim.meta->move[move_no].num_frames;
}

// edit skelettal animation via script
void Model::begin_edit_animation() {
	if (!anim.meta)
		return;
	anim.num_operations = -2;
	for (int i=0;i<bone.num;i++){
		bone[i].cur_ang = quaternion::ID;
		bone[i].cur_pos = bone[i].delta_pos;
	}
}

// make sure, the model/skin is editable and not just a reference.... (soon)
void Model::begin_edit(int detail) {
	make_editable();
	if (mesh[detail]->owner != this) {
		mesh[detail] = mesh[detail]->copy(this);
	}
}

// force an update for this model/skin
void Model::end_edit(int detail) {
	mesh[detail]->update_vb();
	//for (int i=0; i<material.num; i++)
	//	mesh[detail]->sub[i].force_update = true;
}

// make sure we can edit this object without destroying an original one
void Model::make_editable() {
}

Path Model::filename() {
	if (_template)
		return _template->filename;
	return "?";
}

#if 0
void Model::SortingTest(vector &delta_pos,const vector &dpos,matrix *mat,bool allow_shadow)
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
	MetaAddSorted(this,delta_pos,mat,_Detail_,ld,trans,allow_shadow);
}
#endif

