/*
 * DataModel.cpp
 *
 *  Created on: 04.03.2012
 *      Author: michi
 */

#include "DataModel.h"
#include "ModelMesh.h"
#include "ModelMaterial.h"
//#include "ModelSelection.h"
#include "../../data/mesh/PolygonMesh.h"
#if 0 //HAS_LIB_GL
#include "../../mode/model/ModeModel.h"
#include "../../mode/model/mesh/ModeModelMesh.h"
#endif
#include "../../action/Action.h"
#include "../../action/ActionManager.h"
#include "../../Session.h"
#include "../../storage/Storage.h"
#include "../../lib/math/Box.h"
#if 0 //HAS_LIB_GL
#include "../../multiview/MultiView.h"
#endif
/*#include "../../action/model/mesh/vertex/ActionModelAddVertex.h"
#include "../../action/model/mesh/vertex/ActionModelNearifyVertices.h"
#include "../../action/model/mesh/vertex/ActionModelCollapseVertices.h"
#include "../../action/model/mesh/vertex/ActionModelAlignToGrid.h"
#include "../../action/model/mesh/vertex/ActionModelFlattenVertices.h"
#include "../../action/model/mesh/vertex/ActionModelTriangulateVertices.h"
#include "../../action/model/mesh/edge/ActionModelBevelEdges.h"
#include "../../action/model/mesh/polygon/ActionModelAddPolygonSingleTexture.h"
#include "../../action/model/mesh/polygon/ActionModelExtrudePolygons.h"
#include "../../action/model/mesh/polygon/ActionModelTriangulateSelection.h"
#include "../../action/model/mesh/polygon/ActionModelMergePolygonsSelection.h"
#include "../../action/model/mesh/polygon/ActionModelCutOutPolygons.h"
#include "../../action/model/mesh/surface/helper/ActionModelSurfaceDeletePolygon.h"
#include "../../action/model/mesh/surface/ActionModelSurfaceVolumeSubtract.h"
#include "../../action/model/mesh/surface/ActionModelSurfaceVolumeAnd.h"
#include "../../action/model/mesh/surface/ActionModelSurfaceInvert.h"
#include "../../action/model/mesh/surface/ActionModelAutoWeldSelection.h"
#include "../../action/model/mesh/surface/ActionModelSurfacesSubdivide.h"
#include "../../action/model/mesh/skin/ActionModelAutomap.h"*/
#include "../mesh/action/look/ActionModelSetMaterial.h"
/*#include "../../action/model/mesh/look/ActionModelSetNormalModeSelection.h"
#include "../../action/model/mesh/effect/ActionModelAddEffects.h"
#include "../../action/model/mesh/effect/ActionModelClearEffects.h"
#include "../../action/model/mesh/effect/ActionModelEditEffect.h"
#include "../../action/model/mesh/ActionModelDeleteSelection.h"
#include "../../action/model/mesh/ActionModelPasteGeometry.h"
#include "../../action/model/mesh/ActionModelEasify.h"
#include "../../action/model/animation/ActionModelAddAnimation.h"
#include "../../action/model/animation/ActionModelDuplicateAnimation.h"
#include "../../action/model/animation/ActionModelDeleteAnimation.h"
#include "../../action/model/animation/ActionModelAnimationAddFrame.h"
#include "../../action/model/animation/ActionModelAnimationDeleteFrame.h"
#include "../../action/model/animation/ActionModelAnimationSetData.h"
#include "../../action/model/animation/ActionModelAnimationSetFrameDuration.h"
#include "../../action/model/animation/ActionModelAnimationSetBone.h"
#include "../../action/model/skeleton/ActionModelAddBone.h"
#include "../../action/model/skeleton/ActionModelAttachVerticesToBone.h"
#include "../../action/model/skeleton/ActionModelDeleteBone.h"
#include "../../action/model/skeleton/ActionModelDeleteBoneSelection.h"
#include "../../action/model/skeleton/ActionModelReconnectBone.h"
#include "../../action/model/skeleton/ActionModelSetSubModel.h"*/
#include "../mesh/action/ActionModelAddPolygon.h"
#include "../mesh/action/ActionModelDeleteSelection.h"
#include "../mesh/action/ActionModelPasteMesh.h"
#include "../mesh/action/ActionModelEditMesh.h"
#include "../../lib/os/msg.h"
#include "../../lib/math/quaternion.h"
#include "../../lib/xhui/language.h"
#include <y/helper/ResourceManager.h>
#include <y/world/components/Animator.h>
#include <lib/ygraphics/graphics-impl.h>
#include <lib/base/iter.h>


string ModelEffect::get_type()
{
	if (type == FX_TYPE_SCRIPT)
		return _("Script");
	if (type == FX_TYPE_LIGHT)
		return _("Light");
	if (type == FX_TYPE_SOUND)
		return _("Sound");
	if (type == FX_TYPE_FORCEFIELD)
		return _("Forcefield");
	return "???";
}

void ModelEffect::clear()
{
	file = "";
	colors[0] = White;
	colors[1] = White;
	colors[2] = White;
	size = 1000.0f;
	speed = 1.0f;
	intensity = 100.0f;
	inv_quad = false;
}


DataModel::DataModel(Session *s) :
	Data(s, FD_MODEL)
{
	mesh = new ModelMesh();
	phys_mesh = new ModelMesh();
	editing_mesh = mesh.get();
	triangle_mesh.resize(4);
}

DataModel::~DataModel() = default;


void DataModel::MetaData::reset() {
	// level of detail
	detail_dist[0] = 2000;
	detail_dist[1] = 4000;
	detail_dist[2] = 8000;
	auto_generate_dists = true;
	detail_factor[1] = 40;
	detail_factor[2] = 20;
	auto_generate_skin[1] = false;//true;
	auto_generate_skin[2] = false;//true;

	// physics
	mass = 1;
	active_physics = true;
	passive_physics = true;
	auto_generate_tensor = true;
	inertia_tensor = mat3::ID;

	// object data
	name = "";
	description = "";
	inventary.clear();

	// script
	script_file = "";
	script_var.clear();
	variables.clear();
}



void DataModel::reset() {

	filename = "";
	for (int i=0;i<4;i++) {
		triangle_mesh[i].vertices.clear();
		for (int j=0;j<materials.num;j++)
			triangle_mesh[i].sub[j].triangles.clear();
		triangle_mesh[i].sub.resize(1);
	}

	mesh->clear();
	phys_mesh->clear();

	fx.clear();

	materials.clear();
	materials.resize(1);
	materials[0] = new ModelMaterial(session);
	materials[0]->texture_levels.add(new ModelMaterial::TextureLevel);
	materials[0]->texture_levels[0]->reload_image(session);
	materials[0]->col.user = true;
	materials[0]->col.albedo = White;
	materials[0]->col.roughness = 0.4f;

	// skeleton
	bones.clear();

	moves.clear();


	for (int i=0;i<4;i++){
		triangle_mesh[i].sub.resize(1);
		triangle_mesh[i].sub[0].num_textures = 1;
	}

	meta_data.reset();

	reset_history();
	out_changed();
}

void DataModel::debug_show() {
	msg_write("------------");
	msg_write(mesh->vertices.num);
	msg_write(mesh->polygons.num);
	//test_sanity("Model.DebugShow");
}

/*bool DataModel::test_sanity(const string &loc) {
	return mesh->test_sanity(loc);
}*/


/*void DataModel::on_post_action_update() {
	mesh->on_post_action_update();
	phys_mesh->on_post_action_update();
}*/


void DataModel::import_from_triangle_mesh(int index) {
#if 0
	mesh->vertex.clear();
	mesh->polygon.clear();
	mesh->edge.clear();
	action_manager->enable(false);

	ModelTriangleMesh &s = triangle_mesh[index];
	begin_action_group("ImportFromTriangleSkin");
	foreachi(ModelVertex &v, s.vertex, i) {
		addVertex(v.pos, v.bone_index, v.bone_weight);
	}
	for (int i=0;i<material.num;i++) {
		for (ModelTriangle &t: s.sub[i].triangle) {
			if ((t.vertex[0] == t.vertex[1]) || (t.vertex[1] == t.vertex[2]) || (t.vertex[2] == t.vertex[0]))
				continue;
			Array<int> v;
			for (int k=0;k<3;k++)
				v.add(t.vertex[k]);
			Array<vec3> sv;
			for (int tl=0;tl<material[i]->texture_levels.num;tl++)
				for (int k=0;k<3;k++)
					sv.add(t.skin_vertex[tl][k]);
			try {
				addPolygonWithSkin(v, sv, i);
			} catch(GeometryException &e) {
				msg_error("polygon..." + e.message);
				msg_write("trying to copy vertices...");

				// copy all vertices m(-_-)m
				int nv0 = mesh->vertex.num;
				for (int k=0; k<3; k++) {
					addVertex(mesh->vertex[v[k]].pos, mesh->vertex[v[k]].bone_index, mesh->vertex[v[k]].bone_weight);
					v[k] = nv0 + k;
				}
				addPolygonWithSkin(v, sv, i);
			}
		}
	}
	action_manager->enable(true);

#if 0
	ModelTriangleMesh &ps = triangle_mesh[0];
	foreachi(ModelVertex &v, ps.vertex, i){
		phys_mesh->addVertex(v.pos, v.bone_index);
	}
	for (ModelPolyhedron &p: polyhedron){
		msg_write("----");
		int nv0 = vertex.num;
		Array<int> vv;
		for (int i=0;i<p.NumFaces;i++){
			Array<int> v;
			for (int j=0;j<p.Face[i].NumVertices;j++){
				int vj = p.Face[i].Index[j];
				bool existing = false;
				for (int k=0;k<vv.num;k++)
					if (vv[k] == vj){
						existing = true;
						v.add(nv0 + k);
						break;
					}
				if (!existing){
					v.add(vertex.num);
					vv.add(vj);
					addVertex(ps.vertex[vj].pos);
					vertex.back().bone_index = ps.vertex[vj].bone_index;
				}
			}
			msg_write(ia2s(v));
			try{
				addPolygon(v, 0);
			}catch(GeometryException &e){
				msg_error(e.message);
			}
		}
		//surface.back().is_physical = true;
		//surface.back().is_visible = false;
	}
	polyhedron.clear();
#endif

	//clearSelection();
	end_action_group();
	action_manager->reset();
#endif
}


Box DataModel::bounding_box() {
	auto box = mesh->bounding_box();
	auto box2 = phys_mesh->bounding_box();
	if (box2.min != box2.max)
		box = box or box2;
	return box;
}

/*void DataModel::set_normals_dirty_by_vertices(const Array<int> &index) {
	mesh->set_normals_dirty_by_vertices(index);
}

void DataModel::set_all_normals_dirty() {
	mesh->set_all_normals_dirty();
	phys_mesh->set_all_normals_dirty();
}


void DataModel::update_normals() {
	mesh->update_normals();
	phys_mesh->update_normals();
}*/

#if 0
void DataModel::clearSelection() {
	mesh->clear_selection();
	phys_mesh->clear_selection();
}

void DataModel::selectionFromPolygons() {
	mesh->selection_from_polygons();
}

void DataModel::selectionFromEdges() {
	mesh->selection_from_edges();
}

void DataModel::selectionFromVertices() {
	mesh->selection_from_vertices();
}
#endif

void DataModel::add_vertex(const vec3 &pos, const ivec4 &bone_index, const vec4 &bone_weight, int normal_mode) {
	PolygonMesh m;
	m.add_vertex(pos);
	m.vertices[0].bone_index = bone_index;
	m.vertices[0].bone_weight = bone_weight;
	m.vertices[0].normal_mode = normal_mode;
	paste_mesh(m, 0);
	//execute(new ActionModelAddVertex(pos, bone_index, bone_weight, normal_mode));
}

Polygon *DataModel::add_triangle(int a, int b, int c, int material) {
	return add_polygon({a, b, c}, material);
}

Polygon *DataModel::add_polygon(const Array<int> &v, int material) {
	Array<vec3> sv;
	for (int i=0;i<v.num;i++) {
		float w = (float)i / (float)v.num * 2 * pi;
		sv.add(vec3(0.5f + cos(w) * 0.5f, 0.5f + sin(w), 0));
	}
	return add_polygon_with_skin(v, sv, material);
}

Polygon *DataModel::add_polygon_with_skin(const Array<int> &v, const Array<vec3> &sv, int material) {
	Polygon p;
	for (int i=0; i<v.num; i++) {
		PolygonSide s;
		s.vertex = v[i];
		for (int k=0; k<MATERIAL_MAX_TEXTURES; k++)
			s.skin_vertex[k] = sv[k];
		s.normal_index = -1;
		s.smoothing_id = -1;
		p.side.add(s);
		p.material = material;
	}
	return (Polygon*)execute(new ActionModelAddPolygon(editing_mesh, p));
}

void DataModel::edit_mesh(const MeshEdit& edit) {
	execute(new ActionModelEditMesh(editing_mesh, edit));
}



#if 0
void DataModel::create_triangle_mesh(ModelTriangleMesh *src, ModelTriangleMesh *dst, float quality_factor) {
	msg_todo("DataModel.create_triangle_mesh");
}
#endif

#if 0
Data::Selection DataModel::get_selection() const {
	Selection sel;
	sel.add({MultiViewType::MODEL_VERTEX, {}});
	sel.add({MultiViewType::MODEL_POLYGON, {}});
	sel.add({MultiViewType::MODEL_BALL, {}});
	sel.add({MultiViewType::MODEL_CYLINDER, {}});
	for (const auto& [i, v]: enumerate(editing_mesh->vertices))
		if (v.is_selected)
			sel[MultiViewType::MODEL_VERTEX].add(i);
	for (const auto& [i, p]: enumerate(editing_mesh->polygons))
		if (p.is_selected)
			sel[MultiViewType::MODEL_POLYGON].add(i);
	for (const auto& [i, b]: enumerate(editing_mesh->spheres))
		if (b.is_selected)
			sel[MultiViewType::MODEL_BALL].add(i);
	for (const auto& [i, c]: enumerate(editing_mesh->cylinders))
		if (c.is_selected)
			sel[MultiViewType::MODEL_CYLINDER].add(i);
	return sel;
}
#endif


float DataModel::get_radius() {
	float radius = 0;
	for (auto &v: mesh->vertices)
		radius = max(v.pos.length(), radius);
	return radius;
}

float DetailDistTemp1,DetailDistTemp2,DetailDistTemp3;

int get_num_trias(DataModel *m, ModelTriangleMesh *s) {
	int n = 0;
	for (int i=0;i<m->materials.num;i++)
		n += s->sub[i].triangles.num;
	return n;
}

void DataModel::generateDetailDists(float *dist) {
	float radius = get_radius();
	dist[0] = radius * 10;
	dist[1] = radius * 40;
	dist[2] = radius * 80;
	if (get_num_trias(this, &triangle_mesh[3]) == 0)
		dist[1] = dist[2];
	if (get_num_trias(this, &triangle_mesh[2]) == 0)
		dist[0] = dist[1];
}


#define n_theta		16

mat3 DataModel::generateInertiaTensor(float mass)
{
//	sModeModelSkin *p = &Skin[0];

	// estimate size
	auto box = mesh->bounding_box();
	/*for (int i=0;i<Ball.num;i++){
		sModeModelBall *b = &Ball[i];
		vector b_min = p->Vertex[b->Index].Pos - vector(1,1,1) * b->Radius;
		vector b_max = p->Vertex[b->Index].Pos + vector(1,1,1) * b->Radius;
		VecMin(min, b_min);
		VecMax(max, b_max);
	}*/
	//msg_write(string2("	min= %f	%f	%f",min.x,min.y,min.z));
	//msg_write(string2("	max= %f	%f	%f",max.x,max.y,max.z));


	//float dv=(max.x-min.x)/n_theta*(max.y-min.y)/n_theta*(max.z-min.z)/n_theta;
	int num_ds=0;

	mat3 t = mat3::ZERO;

#if 0
	mesh->begin_inside_tests();

	for (int i=0;i<n_theta;i++){
		float x=min.x+(float(i)+0.5f)*(max.x-min.x)/n_theta;
		for (int j=0;j<n_theta;j++){
			float y=min.y+(float(j)+0.5f)*(max.y-min.y)/n_theta;
			for (int k=0;k<n_theta;k++){
				float z=min.z+(float(k)+0.5f)*(max.z-min.z)/n_theta;
				vec3 r=vec3(x,y,z);
				//msg_write(string2("%f		%f		%f",r.x,r.y,r.z));

				bool inside=false;
				/*for (int n=0;n<Ball.num;n++){
					sModeModelBall *b=&Ball[n];
					if (VecLength(r-p->Vertex[b->Index].Pos)<b->Radius)
						inside=true;
				}*/
				if (mesh->inside_test(r)){
					inside = true;
					break;
				}
				if (inside){
					//msg_write("in");
					num_ds++;
					t._00 += y*y + z*z;
					t._11 += z*z + x*x;
					t._22 += x*x + y*y;
					t._01 -= x*y;
					t._12 -= y*z;
					t._20 -= z*x;
				}
			}
		}
	}


	mesh->end_inside_tests();

	if (num_ds>0){
		float f = mass / num_ds;
		t *= f;
		t._10 = t._01;
		t._21 = t._12;
		t._02 = t._20;
	}else
		t = mat3::ID;
#endif

	return t;
}

#if 0
void DataModel::reconnectBone(int index, int parent)
{	execute(new ActionModelReconnectBone(index, parent));	}

void DataModel::setBoneModel(int index, const Path &filename)
{	execute(new ActionModelSetSubModel(index, filename, session->resource_manager->load_model(filename)));	}

void DataModel::addBone(const vec3 &pos, int parent)
{	execute(new ActionModelAddBone(pos, parent));	}

void DataModel::deleteBone(int index)
{	execute(new ActionModelDeleteBone(index));	}

void DataModel::deleteSelectedBones()
{	execute(new ActionModelDeleteBoneSelection(this));	}

void DataModel::boneAttachVertices(const Array<int> &vertices, const Array<ivec4> &bone, const Array<vec4> &weight)
{	execute(new ActionModelAttachVerticesToBone(vertices, bone, weight));	}

void DataModel::addAnimation(int index, AnimationType type)
{	execute(new ActionModelAddAnimation(index, type));	}

void DataModel::duplicateAnimation(int source, int target)
{	execute(new ActionModelDuplicateAnimation(source, target));	}

void DataModel::deleteAnimation(int index)
{	execute(new ActionModelDeleteAnimation(index));	}

void DataModel::animationAddFrame(int index, int frame, const ModelFrame &f)
{	execute(new ActionModelAnimationAddFrame(index, frame, f));	}

void DataModel::setAnimationData(int index, const string &name, float fps_const, float fps_factor)
{	execute(new ActionModelAnimationSetData(index, name, fps_const, fps_factor));	}

void DataModel::animationDeleteFrame(int index, int frame)
{	execute(new ActionModelAnimationDeleteFrame(index, frame));	}

void DataModel::animationSetFrameDuration(int index, int frame, float duration)
{	execute(new ActionModelAnimationSetFrameDuration(index, frame, duration));	}

void DataModel::animationSetBone(int move, int frame, int bone, const vec3 &dpos, const vec3 &ang)
{	execute(new ActionModelAnimationSetBone(move, frame, bone, dpos, ang));	}
#endif

void DataModel::delete_selection(const Selection& s, bool greedy) {
	execute(new ActionModelDeleteSelection(editing_mesh, s, greedy));
}

#if 0
void DataModel::delete_polygon(int index)
{	execute(new ActionModelSurfaceDeletePolygon(index));	}

void DataModel::invert_polygons(const ModelSelection &s) {
	execute(new ActionModelSurfaceInvert(s.polygon, s.consistent_surfaces(mesh)));
}

void DataModel::subtractSelection(int view_stage)
{	execute(new ActionModelSurfaceVolumeSubtract(view_stage));	}

void DataModel::andSelection()
{	execute(new ActionModelSurfaceVolumeAnd());	}

void DataModel::alignToGridSelection(float grid_d)
{	execute(new ActionModelAlignToGrid(this, grid_d));	}

void DataModel::nearifySelectedVertices()
{	execute(new ActionModelNearifyVertices(this));	}

void DataModel::collapseSelectedVertices()
{	execute(new ActionModelCollapseVertices());	}

void DataModel::setNormalModeSelection(int mode)
{	execute(new ActionModelSetNormalModeSelection(this, mode));	}
#endif

void DataModel::apply_material(const Selection& sel, int material) {
	execute(new ActionModelSetMaterial(this, sel, material));
}

void DataModel::paste_mesh(const PolygonMesh& geo, int default_material) {
	execute(new ActionModelPasteMesh(editing_mesh, geo, default_material));
}

#if 0
void DataModel::easify(float factor)
{	execute(new ActionModelEasify(factor));	}

void DataModel::subdivideSelectedSurfaces(const ModelSelection &s) {
	execute(new ActionModelSurfacesSubdivide(s));
}

void DataModel::bevelSelectedEdges(float radius) {
#if HAS_LIB_GL
	execute(new ActionModelBevelEdges(radius, session->mode_model->mode_model_mesh->current_material));
#endif
}

void DataModel::flattenSelectedVertices()
{	execute(new ActionModelFlattenVertices(this));	}

void DataModel::triangulateSelectedVertices()
{	execute(new ActionModelTriangulateVertices());	}

void DataModel::extrudeSelectedPolygons(float offset, bool independent) {
#if HAS_LIB_GL
	execute(new ActionModelExtrudePolygons(offset, independent, session->mode_model->mode_model_mesh->current_material));
#endif
}

void DataModel::autoWeldSurfaces(const base::set<int> &surfaces, float epsilon)
{	execute(new ActionModelAutoWeldSelection(epsilon));	}

void DataModel::autoWeldSelectedSurfaces(float epsilon)
{	execute(new ActionModelAutoWeldSelection(epsilon));	}

void DataModel::automap(int material, int texture_level)
{	execute(new ActionModelAutomap(material, texture_level));	}

void DataModel::selectionAddEffects(const ModelEffect& effect)
{	execute(new ActionModelAddEffects(this, effect));	}

void DataModel::editEffect(int index, const ModelEffect& effect)
{	execute(new ActionModelEditEffect(index, effect));	}

void DataModel::cutOutSelection()
{	execute(new ActionModelCutOutPolygons());	}

void DataModel::convertSelectionToTriangles()
{	execute(new ActionModelTriangulateSelection());	}

void DataModel::mergePolygonsSelection()
{	execute(new ActionModelMergePolygonsSelection());	}

void DataModel::selectionClearEffects()
{	execute(new ActionModelClearEffects(this));	}


ModelSelection DataModel::get_selection() const {
	auto s = mesh->get_selection();
	foreachi (auto &b, bone, i)
		if (b.is_selected)
			s.bone.add(i);
	return s;
}

void DataModel::set_selection(const ModelSelection &s) {
	mesh->set_selection(s);
}
#endif


float ModelMove::duration()
{
	float t = 0;
	for (ModelFrame &f: frames)
		t += f.duration;
	return t;
}

bool ModelMove::needsRubberTiming()
{
	for (ModelFrame &f: frames)
		if (fabs(f.duration - 1.0f) > 0.01f)
			return true;
	return false;
}

void ModelMove::getTimeInterpolation(float time, int &frame0, int &frame1, float &t)
{
	float t0 = 0;
	foreachi(ModelFrame &f, frames, i){
		if (time < t0 + f.duration){
			frame0 = i;
			frame1 = (i + 1) % frames.num;
			t = (time - t0) / f.duration;
			return;
		}
		t0 += f.duration;
	}
	frame0 = 0;
	frame1 = 0;
	t = 0;
}

ModelFrame ModelMove::interpolate(float time)
{
	ModelFrame f;
	float t;
	int frame0, frame1;
	getTimeInterpolation(time, frame0, frame1, t);
	ModelFrame &f0 = frames[frame0];
	ModelFrame &f1 = frames[frame1];

	if (type == AnimationType::VERTEX){
		int n = f0.vertex_dpos.num;
		f.vertex_dpos.resize(n);
		for (int i=0; i<n; i++)
			f.vertex_dpos[i] = (1 - t) * f0.vertex_dpos[i] + t * f1.vertex_dpos[i];
	}else if (type == AnimationType::SKELETAL){
		int n = f0.skel_ang.num;
		f.skel_ang.resize(n);
		f.skel_dpos.resize(n);
		for (int i=0; i<n; i++){
			f.skel_dpos[i] = (1 - t) * f0.skel_dpos[i] + t * f1.skel_dpos[i];
			quaternion q0, q1, q;
			q0 = quaternion::rotation_v( f0.skel_ang[i]);
			q1 = quaternion::rotation_v( f1.skel_ang[i]);
			q = quaternion::interpolate( q0, q1, t);
			f.skel_ang[i] = q.get_angles();
		}
	}
	return f;
}
