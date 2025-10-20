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
#include "ModelManager.h"
#include <lib/yrenderer/Material.h>
#include "World.h"
#include <ecs/Entity.h>
#include <lib/math/complex.h>
#include "../meta.h"
#include <lib/ygraphics/graphics-impl.h>
#include <lib/os/msg.h>
#include "components/Animator.h"


#define DynamicNormalCorrect

const kaba::Class* ModelRef::_class = nullptr;
const kaba::Class* Model::_class = nullptr;




bool Model::AllowDeleteRecursive = true;




SubMesh::SubMesh() {
	force_update = true;
	num_triangles = 0;
	vertex_buffer = nullptr;
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
	bool using_animation = false;
	for (const auto& c: owner->_template->components)
		if (c.class_name == "Animator")
			using_animation = true;
	s->create_vb(using_animation);
	s->update_vb(using_animation);

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
void Model::_ResetPhysAbsolute_() {
#if 0
	auto col = (MeshCollider*)get_component(MeshCollider::_class);
	if (col)
		col->phys_absolute.is_ok=false;
	for (int i=0;i<bone.num;i++)
		if (bone[i].model)
			bone[i].model->_ResetPhysAbsolute_();
#endif
}



// TODO: move this into the renderer
void SubMesh::create_vb(bool animated) {
	if (animated)
		vertex_buffer = new ygfx::VertexBuffer("3f,3f,2f,4i,4f");
	else
		vertex_buffer = new ygfx::VertexBuffer("3f,3f,2f");
}
void Mesh::create_vb(bool animated) {
	for (auto &s: sub)
		s.create_vb(animated);
}

struct VertexAnimated {
	vec3 p, n;
	float u,v;
	ivec4 bone;
	vec4 weight;
};

void SubMesh::update_vb(Mesh *mesh, bool animated) {
#if 0
	if (true) {
		Array<vec3> p, n;
		Array<complex> uv;
		for (int i=0; i<num_triangles; i++) {
			for (int k=0; k<3; k++) {
				int vi = triangle_index[i*3+k];
				p.add(mesh->vertex[vi]);
				n.add(normal[i*3+k]);
				uv.add(complex(skin_vertex[i*6+k*2  ], skin_vertex[i*6+k*2+1]));
			}
		}
		vertex_buffer->update(0, p);
		vertex_buffer->update(1, n);
		vertex_buffer->update(2, uv);
	} else {
		// no... only mesh->vertex is properly indexed right now.... (-_-)'
		vertex_buffer->update_index(triangle_index);
		vertex_buffer->update(0, mesh->vertex);
		vertex_buffer->update(1, normal);
		vertex_buffer->update(2, skin_vertex);
	}
#endif

	if (animated) {
		Array<VertexAnimated> vertex;
		for (int i=0; i<num_triangles; i++) {
			for (int k=0; k<3; k++) {
				int vi = triangle_index[i*3+k];
				auto bi = mesh->bone_index[vi];
				for (int l=0; l<4; l++)
					bi[l] = max(bi[l], 0);
				vertex.add({mesh->vertex[vi], normal[i*3+k], skin_vertex[i*6+k*2  ], skin_vertex[i*6+k*2+1], bi, mesh->bone_weight[vi]});
			}
		}
		vertex_buffer->update(vertex);
	} else {
		Array<ygfx::Vertex1> vertex;
		for (int i=0; i<num_triangles; i++) {
			for (int k=0; k<3; k++) {
				int vi = triangle_index[i*3+k];
				vertex.add({mesh->vertex[vi], normal[i*3+k], skin_vertex[i*6+k*2  ], skin_vertex[i*6+k*2+1]});
			}
		}
		vertex_buffer->update(vertex);
	}
}

void Mesh::update_vb(bool animated) {
	for (auto &s: sub)
		s.update_vb(this, animated);
}

void Mesh::post_process(bool animated) {

	create_vb(animated);
	update_vb(animated);

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


void Model::reset_data() {
	for (int i=0; i<MODEL_NUM_MESHES; i++) {
		_detail_needed_[i] = false;
	}

	prop.allow_shadow = false;
}



Model::Model() {
	component_type = Model::_class;

	visible = true;

	is_copy = false;

	for (int i=0;i<MODEL_NUM_MESHES;i++) {
		_detail_needed_[i] = false;
		mesh[i] = nullptr;
	}
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
	m->_template = _template;

	for (auto mat: material)
		m->material.add(mat->copy());
	

	// "copy" presettings (just using references)
	m->is_copy = true;
	for (int i=0; i<MODEL_NUM_MESHES; i++) {
		m->mesh[i] = mesh[i];
		m->_detail_needed_[i] = false;
	}
	m->visible = true;

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

	// skin
	/*for (int i=0;i<MODEL_NUM_MESHES;i++)
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
		delete m;*/
}

void Model::__delete__() {
	this->Model::~Model();
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


#if 0

float line_dist(const vec3 &pa, const vec3 &da, const vec3 &pb, const vec3 &db, vec3 &pointa, vec3 &pointb)
{
	plane pl;
	vec3 n = da ^ db; // shortest direction
	n.normalize();
	pl = plane::from_point_normal( pb, n ^ db); // line b in pl
	pl.intersect_line(pa, pa + da, pointa);
	float d = VecLineDistance(pointa, pb, pb + db);
	vec3 x1 = pointa - n * d;
	vec3 x2 = pointa + n * d;
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
bool Model::Trace(const vec3 &p1, const vec3 &p2, const vec3 &dir, float range, TraceData &data, bool simple_test)
{
	if (!physics_data.passive)
		return false;
	
	bool hit=false;
	vec3 c;
	float dmin=range;
	// skeleton -> recursion
	if (bone.num>0){
		vec3 c;
		for (int i=0;i<bone.num;i++)
			if (bone[i].model){
				vec3 p2t=p1+dmin*p2;
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
	vec3 tm = (p1+p2)/2; // Mittelpunkt des Trace-Strahles
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
	vec3 *p = &phys_absolute.p[0];

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
		vec3 &ca = p[c.index[0]];
		vec3 &cb = p[c.index[1]];
		vec3 cd = cb - ca;
		cd.normalize();
		vec3 cp, tp;
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


bool Model::TraceMesh(const vec3 &p1, const vec3 &p2, const vec3 &dir, float range, TraceData &data, bool simple_test)
{
	return false;
}
#endif


vec3 _cdecl Model::get_vertex(int index) {
	auto s = mesh[MESH_HIGH];
	return _matrix * s->vertex[index];
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
	mesh[detail]->update_vb(owner->get_component<Animator>());
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

void Model::update_matrix() {
	if (owner)
		_matrix = owner->get_matrix();
}

#if 0
void Model::SortingTest(vec3 &delta_pos,const vec3 &dpos,matrix *mat,bool allow_shadow)
{
	for (int i=0;i<bone.num;i++)
		if (boneModel[i]){
			vec3 sub_pos;
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

