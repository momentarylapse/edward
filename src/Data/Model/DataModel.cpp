/*
 * DataModel.cpp
 *
 *  Created on: 04.03.2012
 *      Author: michi
 */

#include "DataModel.h"
#include "ModelMesh.h"
#include "ModelPolygon.h"
#include "ModelMaterial.h"
#include "ModelSelection.h"
#include "Geometry/Geometry.h"
#include "../../Mode/Model/ModeModel.h"
#include "../../Action/Action.h"
#include "../../Action/ActionManager.h"
#include "../../Edward.h"
#include "../../Storage/Storage.h"
#include "../../MultiView/MultiView.h"
#include "../../Action/Model/Mesh/Vertex/ActionModelAddVertex.h"
#include "../../Action/Model/Mesh/Vertex/ActionModelNearifyVertices.h"
#include "../../Action/Model/Mesh/Vertex/ActionModelCollapseVertices.h"
#include "../../Action/Model/Mesh/Vertex/ActionModelAlignToGrid.h"
#include "../../Action/Model/Mesh/Vertex/ActionModelFlattenVertices.h"
#include "../../Action/Model/Mesh/Vertex/ActionModelTriangulateVertices.h"
#include "../../Action/Model/Mesh/Edge/ActionModelBevelEdges.h"
#include "../../Action/Model/Mesh/Polygon/ActionModelAddPolygonSingleTexture.h"
#include "../../Action/Model/Mesh/Polygon/ActionModelExtrudePolygons.h"
#include "../../Action/Model/Mesh/Polygon/ActionModelTriangulateSelection.h"
#include "../../Action/Model/Mesh/Polygon/ActionModelMergePolygonsSelection.h"
#include "../../Action/Model/Mesh/Polygon/ActionModelCutOutPolygons.h"
#include "../../Action/Model/Mesh/Surface/Helper/ActionModelSurfaceDeletePolygon.h"
#include "../../Action/Model/Mesh/Surface/ActionModelSurfaceVolumeSubtract.h"
#include "../../Action/Model/Mesh/Surface/ActionModelSurfaceVolumeAnd.h"
#include "../../Action/Model/Mesh/Surface/ActionModelSurfaceInvert.h"
#include "../../Action/Model/Mesh/Surface/ActionModelAutoWeldSelection.h"
#include "../../Action/Model/Mesh/Surface/ActionModelSurfacesSubdivide.h"
#include "../../Action/Model/Mesh/Skin/ActionModelAutomap.h"
#include "../../Action/Model/Mesh/Look/ActionModelSetMaterial.h"
#include "../../Action/Model/Mesh/Look/ActionModelSetNormalModeSelection.h"
#include "../../Action/Model/Mesh/Effects/ActionModelAddEffects.h"
#include "../../Action/Model/Mesh/Effects/ActionModelClearEffects.h"
#include "../../Action/Model/Mesh/Effects/ActionModelEditEffect.h"
#include "../../Action/Model/Mesh/ActionModelDeleteSelection.h"
#include "../../Action/Model/Mesh/ActionModelPasteGeometry.h"
#include "../../Action/Model/Mesh/ActionModelEasify.h"
#include "../../Action/Model/Animation/ActionModelAddAnimation.h"
#include "../../Action/Model/Animation/ActionModelDuplicateAnimation.h"
#include "../../Action/Model/Animation/ActionModelDeleteAnimation.h"
#include "../../Action/Model/Animation/ActionModelAnimationAddFrame.h"
#include "../../Action/Model/Animation/ActionModelAnimationDeleteFrame.h"
#include "../../Action/Model/Animation/ActionModelAnimationSetData.h"
#include "../../Action/Model/Animation/ActionModelAnimationSetFrameDuration.h"
#include "../../Action/Model/Animation/ActionModelAnimationSetBone.h"
#include "../../Action/Model/Skeleton/ActionModelAddBone.h"
#include "../../Action/Model/Skeleton/ActionModelAttachVerticesToBone.h"
#include "../../Action/Model/Skeleton/ActionModelDeleteBone.h"
#include "../../Action/Model/Skeleton/ActionModelDeleteBoneSelection.h"
#include "../../Action/Model/Skeleton/ActionModelReconnectBone.h"
#include "../../Action/Model/Skeleton/ActionModelSetSubModel.h"
#include "../../x/ModelManager.h"

const string DataModel::MESSAGE_SKIN_CHANGE = "SkinChange";
const string DataModel::MESSAGE_MATERIAL_CHANGE = "MaterialChange";
const string DataModel::MESSAGE_TEXTURE_CHANGE = "TextureChange";


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

ModelVertex::ModelVertex(const vector &_pos) {
	pos = _pos;
	ref_count = 0;
	normal_mode = NORMAL_MODE_ANGULAR;
	bone_index = -1;
	normal_dirty = false;
}

ModelVertex::ModelVertex() : ModelVertex(v_0) {}

DataModel::DataModel() :
	Data(FD_MODEL)
{
	mesh = new ModelMesh(this);
	phys_mesh = new ModelMesh(this);
	edit_mesh = mesh;
}

DataModel::~DataModel() {
	delete mesh;
	delete phys_mesh;
}


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
	inertia_tensor = matrix3::ID;

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
		skin[i].vertex.clear();
		for (int j=0;j<material.num;j++)
			skin[i].sub[j].triangle.clear();
		skin[i].sub.resize(1);
	}

	mesh->clear();
	phys_mesh->clear();

	fx.clear();

	material.clear();
	material.resize(1);
	material[0] = new ModelMaterial();
	material[0]->texture_levels.add(new ModelMaterial::TextureLevel());
	material[0]->texture_levels[0]->reload_image();
	material[0]->col.user = true;
	material[0]->col.diffuse = White;
	material[0]->col.specular = White;

	// skeleton
	bone.clear();

	move.clear();


	for (int i=0;i<4;i++){
		skin[i].sub.resize(1);
		skin[i].sub[0].num_textures = 1;
	}

	meta_data.reset();

	reset_history();
	notify();
}

void DataModel::debugShow() {
	msg_write("------------");
	msg_write(mesh->vertex.num);
	msg_write(mesh->polygon.num);
	test_sanity("Model.DebugShow");
}

bool DataModel::test_sanity(const string &loc) {
	return mesh->test_sanity(loc);
}


void DataModel::on_post_action_update() {
	mesh->on_post_action_update();
	phys_mesh->on_post_action_update();
}


void DataModel::importFromTriangleSkin(int index) {
	mesh->vertex.clear();
	mesh->polygon.clear();
	mesh->edge.clear();

	ModelSkin &s = skin[index];
	begin_action_group("ImportFromTriangleSkin");
	foreachi(ModelVertex &v, s.vertex, i) {
		addVertex(v.pos, v.bone_index);
	}
	for (int i=0;i<material.num;i++) {
		for (ModelTriangle &t: s.sub[i].triangle) {
			if ((t.vertex[0] == t.vertex[1]) || (t.vertex[1] == t.vertex[2]) || (t.vertex[2] == t.vertex[0]))
				continue;
			Array<int> v;
			for (int k=0;k<3;k++)
				v.add(t.vertex[k]);
			Array<vector> sv;
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
					addVertex(mesh->vertex[v[k]].pos, mesh->vertex[v[k]].bone_index);
					v[k] = nv0 + k;
				}
				addPolygonWithSkin(v, sv, i);
			}
		}
	}

#if 0
	ModelSkin &ps = skin[0];
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

	clearSelection();
	end_action_group();
	action_manager->reset();
}


void DataModel::getBoundingBox(vector &min, vector &max) {
	mesh->get_bounding_box(min, max);
	phys_mesh->get_bounding_box(min, max, true);
}

void DataModel::setNormalsDirtyByVertices(const Array<int> &index) {
	mesh->set_normals_dirty_by_vertices(index);
}

void DataModel::setAllNormalsDirty() {
	mesh->set_all_normals_dirty();
	phys_mesh->set_all_normals_dirty();
}


void DataModel::updateNormals() {
	mesh->update_normals();
	phys_mesh->update_normals();
}

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


void DataModel::addVertex(const vector &pos, int bone_index, int normal_mode)
{	execute(new ActionModelAddVertex(pos, bone_index, normal_mode));	}

ModelPolygon *DataModel::addTriangle(int a, int b, int c, int material) {
	return (ModelPolygon*)execute(new ActionModelAddPolygonSingleTexture({a,b,c}, material, {vector::EY, v_0, vector::EX}));
}

ModelPolygon *DataModel::addPolygon(const Array<int> &v, int material)
{
	Array<vector> sv;
	for (int i=0;i<v.num;i++) {
		float w = (float)i / (float)v.num * 2 * pi;
		sv.add(vector(0.5f + cos(w) * 0.5f, 0.5f + sin(w), 0));
	}
	return (ModelPolygon*)execute(new ActionModelAddPolygonSingleTexture(v, material, sv));
}

ModelPolygon *DataModel::addPolygonWithSkin(const Array<int> &v, const Array<vector> &sv, int material) {
	return (ModelPolygon*)execute(new ActionModelAddPolygonSingleTexture(v, material, sv));
}



void DataModel::createSkin(ModelSkin *src, ModelSkin *dst, float quality_factor) {
	msg_todo("DataModel::CreateSkin");
}


float DataModel::getRadius() {
	float radius = 0;
	for (auto &v: mesh->vertex)
		radius = max(v.pos.length(), radius);
	return radius;
}

float DetailDistTemp1,DetailDistTemp2,DetailDistTemp3;

int get_num_trias(DataModel *m, ModelSkin *s) {
	int n = 0;
	for (int i=0;i<m->material.num;i++)
		n += s->sub[i].triangle.num;
	return n;
}

void DataModel::generateDetailDists(float *dist) {
	float radius = getRadius();
	dist[0] = radius * 10;
	dist[1] = radius * 40;
	dist[2] = radius * 80;
	if (get_num_trias(this, &skin[3]) == 0)
		dist[1] = dist[2];
	if (get_num_trias(this, &skin[2]) == 0)
		dist[0] = dist[1];
}


#define n_theta		16

matrix3 DataModel::generateInertiaTensor(float mass)
{
//	sModeModelSkin *p = &Skin[0];

	// estimate size
	vector min, max;
	mesh->get_bounding_box(min, max);
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

	matrix3 t;
	for (int i=0;i<9;i++)
		t.e[i] = 0;

	mesh->begin_inside_tests();

	for (int i=0;i<n_theta;i++){
		float x=min.x+(float(i)+0.5f)*(max.x-min.x)/n_theta;
		for (int j=0;j<n_theta;j++){
			float y=min.y+(float(j)+0.5f)*(max.y-min.y)/n_theta;
			for (int k=0;k<n_theta;k++){
				float z=min.z+(float(k)+0.5f)*(max.z-min.z)/n_theta;
				vector r=vector(x,y,z);
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
		t = matrix3::ID;

	return t;
}

int DataModel::getNumSelectedSkinVertices() {
	int r = 0;
	for (auto &v: mesh->skin_vertex)
		if (v.is_selected)
			r ++;
	return r;
}

void DataModel::reconnectBone(int index, int parent)
{	execute(new ActionModelReconnectBone(index, parent));	}

void DataModel::setBoneModel(int index, const Path &filename)
{	execute(new ActionModelSetSubModel(index, filename));	}

void DataModel::addBone(const vector &pos, int parent)
{	execute(new ActionModelAddBone(pos, parent));	}

void DataModel::deleteBone(int index)
{	execute(new ActionModelDeleteBone(index));	}

void DataModel::deleteSelectedBones()
{	execute(new ActionModelDeleteBoneSelection(this));	}

void DataModel::boneAttachVertices(int index, const Array<int> &vertices)
{	execute(new ActionModelAttachVerticesToBone(vertices, index));	}

void DataModel::addAnimation(int index, int type)
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

void DataModel::animationSetBone(int move, int frame, int bone, const vector &dpos, const vector &ang)
{	execute(new ActionModelAnimationSetBone(move, frame, bone, dpos, ang));	}

void DataModel::copyGeometry(Geometry &geo) {
	geo.clear();

	// copy vertices
	Array<int> vert;
	foreachi(ModelVertex &v, mesh->vertex, vi)
		if (v.is_selected){
			geo.vertex.add(v);
			vert.add(vi);
		}

	// copy triangles
	for (ModelPolygon &t: mesh->polygon)
		if (t.is_selected){
			ModelPolygon tt = t;
			for (int k=0;k<t.side.num;k++)
				foreachi(int v, vert, vi)
					if (v == t.side[k].vertex)
						tt.side[k].vertex = vi;
			geo.polygon.add(tt);
		}
}

void DataModel::delete_selection(const ModelSelection &s, bool greedy)
{	execute(new ActionModelDeleteSelection(s, greedy));	}

void DataModel::delete_polygon(int index)
{	execute(new ActionModelSurfaceDeletePolygon(index));	}

void DataModel::invert_polygons(const ModelSelection &s) {
	execute(new ActionModelSurfaceInvert(s.polygon, s.consistent_surfaces(mesh)));
}

void DataModel::subtractSelection()
{	execute(new ActionModelSurfaceVolumeSubtract());	}

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

void DataModel::setMaterialSelection(int material)
{	execute(new ActionModelSetMaterial(this, material));	}

void DataModel::pasteGeometry(Geometry& geo, int default_material)
{	execute(new ActionModelPasteGeometry(geo, default_material));	}

void DataModel::easify(float factor)
{	execute(new ActionModelEasify(factor));	}

void DataModel::subdivideSelectedSurfaces(const ModelSelection &s) {
	execute(new ActionModelSurfacesSubdivide(s));
}

void DataModel::bevelSelectedEdges(float radius)
{	execute(new ActionModelBevelEdges(radius));	}

void DataModel::flattenSelectedVertices()
{	execute(new ActionModelFlattenVertices(this));	}

void DataModel::triangulateSelectedVertices()
{	execute(new ActionModelTriangulateVertices());	}

void DataModel::extrudeSelectedPolygons(float offset, bool independent)
{	execute(new ActionModelExtrudePolygons(offset, independent));	}

void DataModel::autoWeldSurfaces(const Set<int> &surfaces, float epsilon)
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


float ModelMove::duration()
{
	float t = 0;
	for (ModelFrame &f: frame)
		t += f.duration;
	return t;
}

bool ModelMove::needsRubberTiming()
{
	for (ModelFrame &f: frame)
		if (fabs(f.duration - 1.0f) > 0.01f)
			return true;
	return false;
}

void ModelMove::getTimeInterpolation(float time, int &frame0, int &frame1, float &t)
{
	float t0 = 0;
	foreachi(ModelFrame &f, frame, i){
		if (time < t0 + f.duration){
			frame0 = i;
			frame1 = (i + 1) % frame.num;
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
	ModelFrame &f0 = frame[frame0];
	ModelFrame &f1 = frame[frame1];

	if (type == MOVE_TYPE_VERTEX){
		int n = f0.vertex_dpos.num;
		f.vertex_dpos.resize(n);
		for (int i=0; i<n; i++)
			f.vertex_dpos[i] = (1 - t) * f0.vertex_dpos[i] + t * f1.vertex_dpos[i];
	}else if (type == MOVE_TYPE_SKELETAL){
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
