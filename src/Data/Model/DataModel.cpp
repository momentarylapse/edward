/*
 * DataModel.cpp
 *
 *  Created on: 04.03.2012
 *      Author: michi
 */

#include "DataModel.h"
#include "ModelSurface.h"
#include "Geometry/Geometry.h"
#include "BspTree.h"
#include "../../Mode/Model/ModeModel.h"
#include "../../Action/Action.h"
#include "../../Action/ActionManager.h"
#include "../../Edward.h"
#include "../../Storage/Storage.h"
#include "../../MultiView/MultiView.h"
#include "../../x/model_manager.h"
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

const string DataModel::MESSAGE_SKIN_CHANGE = "SkinChange";
const string DataModel::MESSAGE_MATERIAL_CHANGE = "MaterialChange";
const string DataModel::MESSAGE_TEXTURE_CHANGE = "TextureChange";

bool selection_consistent_surfaces(const ModelSelectionState &s, DataModel *m);


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
	radius = 100;
}

DataModel::~DataModel()
{
}


void DataModel::MetaData::reset()
{
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



void DataModel::reset()
{

	filename = "";
	for (int i=0;i<4;i++){
		skin[i].vertex.clear();
		for (int j=0;j<material.num;j++)
			skin[i].sub[j].triangle.clear();
		skin[i].sub.resize(1);
	}
	polygon.clear();
	edge.clear();
	vertex.clear();
	ball.clear();
	cylinder.clear();
	polyhedron.clear();
	fx.clear();

	material.clear();
	material.resize(1);
	material[0] = new ModelMaterial();
	material[0]->texture_levels.add(new ModelMaterial::TextureLevel());
	material[0]->texture_levels[0]->reload_image();
	material[0]->col.user = true;
	material[0]->col.diffuse = White;
	material[0]->col.specular = White;
	showVertices(vertex);

	// skeleton
	bone.clear();

	move.clear();


	for (int i=0;i<4;i++){
		skin[i].sub.resize(1);
		skin[i].sub[0].num_textures = 1;
	}

	radius = 42;

	meta_data.reset();

	reset_history();
	notify();
}

void DataModel::debugShow()
{
	msg_write("------------");
	msg_write(vertex.num);
	msg_write(polygon.num);
	test_sanity("Model.DebugShow");
}

bool DataModel::test_sanity(const string &loc)
{
	for (ModelPolygon &t: polygon)
		for (int k=0;k<t.side.num;k++)
			for (int kk=k+1;kk<t.side.num;kk++)
				if (t.side[k].vertex == t.side[kk].vertex){
					msg_error(loc + ": surf broken!   identical vertices in poly");
					return false;
				}
	foreachi(ModelEdge &e, edge, i){
		if (e.vertex[0] == e.vertex[1]){
			msg_error(loc + ": surf broken!   trivial edge");
			return false;
		}
		for (int k=0;k<e.ref_count;k++){
			ModelPolygon &t = polygon[e.polygon[k]];
			if (t.side[e.side[k]].edge != i){
				msg_error(loc + ": surf broken!   edge linkage");
				msg_write(format("i=%d  k=%d  side=%d  t.edge=%d t.dir=%d", i, k, e.side[k], t.side[e.side[k]].edge, t.side[e.side[k]].edge_direction));
				return false;
			}
			if (t.side[e.side[k]].edge_direction != k){
				msg_error(loc + ": surf broken!   edge linkage (dir)");
				msg_write(format("i=%d  k=%d  side=%d  t.edge=%d t.dir=%d", i, k, e.side[k], t.side[e.side[k]].edge, t.side[e.side[k]].edge_direction));
				return false;
			}
			for (int j=0;j<2;j++)
				if (e.vertex[(j + k) % 2] != t.side[(e.side[k] + j) % t.side.num].vertex){
					msg_error(loc + ": surf broken!   edge linkage (vert)");
					msg_write(format("i=%d  k=%d  side=%d  t.edge=%d t.dir=%d", i, k, e.side[k], t.side[e.side[k]].edge, t.side[e.side[k]].edge_direction));
					return false;
				}
		}

	}
	return true;
}


void DataModel::on_post_action_update()
{
	showVertices(vertex);

	updateNormals();
	for (ModelPolygon &p: polygon){
		p.pos = v_0;
		for (int k=0;k<p.side.num;k++)
			p.pos += vertex[p.side[k].vertex].pos;
		p.pos /= p.side.num;
	}
	for (ModelEdge &e: edge)
		e.pos = (vertex[e.vertex[0]].pos + vertex[e.vertex[1]].pos) / 2;
}

void DataModel::showVertices(Array<ModelVertex> &vert)
{
	show_vertices.set_ref(vert);
}


int get_normal_index(vector &n);


void report_error(const string &msg)
{
	msg_error(msg);
	if (ed)
		ed->set_message(msg);
}



void DataModel::importFromTriangleSkin(int index)
{
	vertex.clear();
	polygon.clear();
	edge.clear();

	ModelSkin &s = skin[index];
	begin_action_group("ImportFromTriangleSkin");
	foreachi(ModelVertex &v, s.vertex, i){
		addVertex(v.pos);
		vertex[i].bone_index = v.bone_index;
	}
	for (int i=0;i<material.num;i++){
		for (ModelTriangle &t: s.sub[i].triangle){
			if ((t.vertex[0] == t.vertex[1]) || (t.vertex[1] == t.vertex[2]) || (t.vertex[2] == t.vertex[0]))
				continue;
			Array<int> v;
			for (int k=0;k<3;k++)
				v.add(t.vertex[k]);
			Array<vector> sv;
			for (int tl=0;tl<material[i]->texture_levels.num;tl++)
				for (int k=0;k<3;k++)
					sv.add(t.skin_vertex[tl][k]);
			try{
				addPolygonWithSkin(v, sv, i);
			}catch(GeometryException &e){
				msg_error("polygon..." + e.message);
				msg_write("trying to copy vertices...");

				// copy all vertices m(-_-)m
				int nv0 = vertex.num;
				for (int k=0; k<3; k++){
					addVertex(vertex[v[k]].pos);
					vertex.back().bone_index = vertex[v[k]].bone_index;
					v[k] = nv0 + k;
				}
				addPolygonWithSkin(v, sv, i);
			}
		}
	}


	ModelSkin &ps = skin[0];
	foreachi(ModelVertex &v, ps.vertex, i){
		addVertex(v.pos);
		vertex[i].bone_index = v.bone_index;
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

	clearSelection();
	end_action_group();
	action_manager->reset();
}

void DataModel::exportToTriangleSkin(int index)
{
	ModelSkin &sk = skin[index];
	sk.vertex = vertex;
	sk.sub.clear();
	sk.sub.resize(material.num);
	for (ModelPolygon &t: polygon){
		if (t.triangulation_dirty)
			t.updateTriangulation(vertex);
		for (int i=0;i<t.side.num-2;i++){
			ModelTriangle tt;
			for (int k=0;k<3;k++){
				tt.vertex[k] = t.side[t.side[i].triangulation[k]].vertex;
				tt.normal[k] = t.side[t.side[i].triangulation[k]].normal;
				for (int l=0;l<MATERIAL_MAX_TEXTURES;l++)
					tt.skin_vertex[l][k] = t.side[t.side[i].triangulation[k]].skin_vertex[l];
			}
			sk.sub[t.material].triangle.add(tt);
		}
	}
	foreachi(ModelMaterial *m, material, i)
		sk.sub[i].num_textures = m->texture_levels.num;
}


void DataModel::getBoundingBox(vector &min, vector &max)
{
	// bounding box (visual skin[1])
	min = max = v_0;
	for (int i=0;i<skin[1].vertex.num;i++){
		min._min(skin[1].vertex[i].pos);
		max._max(skin[1].vertex[i].pos);
	}
	// (physical skin)
	for (int i=0;i<skin[0].vertex.num;i++){
		min._min(skin[0].vertex[i].pos);
		max._max(skin[0].vertex[i].pos);
	}
	for (int i=0;i<ball.num;i++){
		min._min(skin[0].vertex[ball[i].index].pos - vector(1,1,1) * ball[i].radius);
		max._max(skin[0].vertex[ball[i].index].pos + vector(1,1,1) * ball[i].radius);
	}
}

void DataModel::setNormalsDirtyByVertices(const Array<int> &index)
{
	Set<int> sindex;
	for (int i=0; i<index.num; i++)
		sindex.add(index[i]);

	for (ModelPolygon &t: polygon)
		for (int k=0;k<t.side.num;k++)
			if (!t.normal_dirty)
				if (sindex.contains(t.side[k].vertex)){
					t.normal_dirty = true;
					break;
				}
}

void DataModel::setAllNormalsDirty() {
	for (ModelPolygon &t: polygon)
		t.normal_dirty = true;
}

struct PolySideData {
	int poly;
	int side;
};


inline int find_other_tria_from_edge(DataModel *m, int e, int t) {
	if (m->edge[e].polygon[0] == t)
		return m->edge[e].polygon[1];
	return m->edge[e].polygon[0];
}

// return: closed circle... don't run again to the left
inline bool find_tria_top(DataModel *m, const Array<PolySideData> &pd, Set<int> &used, bool to_the_right) {
	int t0 = 0;
	while(true){
		int side = pd[t0].side;
		if (!to_the_right){
			int ns = m->polygon[pd[t0].poly].side.num;
			side = (side + ns - 1) % ns;
		}
		int e = m->polygon[pd[t0].poly].side[side].edge;
		if (!m->edge[e].is_round)
			return false;
		int tt = find_other_tria_from_edge(m, e, pd[t0].poly);
		if (tt < 0)
			return false;
		t0 = -1;
		for (int i=0;i<pd.num;i++)
			if (pd[i].poly == tt)
				t0 = i;
		if (t0 <= 0)
			return (t0 == 0);
		used.add(t0);
	}
}


void DataModel::updateNormals()
{
	Set<int> ee, vert;

	// "flat" triangle normals
	for (ModelPolygon &t: polygon)
		if (t.normal_dirty){
			t.normal_dirty = false;

			t.temp_normal = t.getNormal(vertex);

			for (int k=0;k<t.side.num;k++){
				t.side[k].normal = t.temp_normal;
				int e = t.side[k].edge;
				if (edge[e].ref_count == 2)
					ee.add(e);
			}
		}

	// round edges?
	for (int ip: ee){
		ModelEdge &e = edge[ip];

		// adjoined triangles
		ModelPolygon &t1 = polygon[e.polygon[0]];
		ModelPolygon &t2 = polygon[e.polygon[1]];

		ModelVertex &v1 = vertex[e.vertex[0]];
		ModelVertex &v2 = vertex[e.vertex[1]];

		// round?
		e.is_round = false;
		if ((v1.normal_mode == NORMAL_MODE_ANGULAR) || (v2.normal_mode == NORMAL_MODE_ANGULAR))
			e.is_round = (t1.temp_normal * t2.temp_normal > 0.6f);

		if (((v1.normal_mode == NORMAL_MODE_ANGULAR) && (e.is_round)) || (v1.normal_mode == NORMAL_MODE_SMOOTH))
			vert.add(e.vertex[0]);
		if (((v2.normal_mode == NORMAL_MODE_ANGULAR) && (e.is_round)) || (v2.normal_mode == NORMAL_MODE_SMOOTH))
			vert.add(e.vertex[1]);

		/*if (e.IsRound){
			vector n = t1.TempNormal + t2.TempNormal;
			VecNormalize(n);
			for (int k=0;k<3;k++)
				if ((t1.Vertex[k] == e.Vertex[0]) || (t1.Vertex[k] == e.Vertex[1]))
					t1.Normal[k] = n;
			for (int k=0;k<3;k++)
				if ((t2.Vertex[k] == e.Vertex[0]) || (t2.Vertex[k] == e.Vertex[1]))
					t2.Normal[k] = n;
		}*/
	}

	// find all triangles shared by each found vertex
	Array<Array<PolySideData> > poly_side;
	poly_side.resize(vert.num);
	foreachi(ModelPolygon &t, polygon, i){
		for (int k=0;k<t.side.num;k++){
			int n = vert.find(t.side[k].vertex);
			if (n >= 0){
				t.side[k].normal = t.temp_normal;
				PolySideData d;
				d.poly = i;
				d.side = k;
				poly_side[n].add(d);
			}
		}
	}

	// per vertex...
	foreachi(int ip, vert, nn){

		// hard vertex -> nothing to do
		if (vertex[ip].normal_mode == NORMAL_MODE_HARD)
			continue;

		Array<PolySideData> &pd = poly_side[nn];

		// smooth vertex
		if (vertex[ip].normal_mode == NORMAL_MODE_SMOOTH){

			// average normal
			vector n = v_0;
			for (int i=0;i<pd.num;i++)
				n += polygon[pd[i].poly].side[pd[i].side].normal;
			n.normalize();
			// apply normal...
			for (int i=0;i<pd.num;i++)
				polygon[pd[i].poly].side[pd[i].side].normal = n;
			continue;
		}

		// angular vertex...

		// find groups of triangles that are connected by round edges
		while (pd.num > 0){

			// start with the 1st triangle
			Set<int> used;
			used.add(0);

			// search to the right
			bool closed = find_tria_top(this, pd, used, true);

			// search to the left
			if (!closed)
				find_tria_top(this, pd, used, false);

			if (used.num == 1){
				// no smoothly connected triangles...
				pd.erase(0);
				continue;
			}

			// average normal
			vector n = v_0;
			for (int i=0;i<used.num;i++)
				n += polygon[pd[used[i]].poly].side[pd[used[i]].side].normal;
			n.normalize();
			// apply normal... and remove from list
			for (int i=used.num-1;i>=0;i--){
				polygon[pd[used[i]].poly].side[pd[used[i]].side].normal = n;
				pd.erase(used[i]);
			}
		}
	}
}


void DataModel::addVertex(const vector &pos, int bone_index, int normal_mode)
{	execute(new ActionModelAddVertex(pos, bone_index, normal_mode));	}

void DataModel::clearSelection()
{
	for (ModelVertex &v: vertex)
		v.is_selected = false;
	for (ModelPolygon &t: polygon)
		t.is_selected = false;
	for (ModelEdge &e: edge)
		e.is_selected = false;
	notify(MESSAGE_SELECTION);
}

void DataModel::selectionFromPolygons() {
	for (ModelVertex &v: vertex)
		v.is_selected = false;
	for (ModelEdge &e: edge)
		e.is_selected = false;
	for (ModelPolygon &t: polygon)
		if (t.is_selected)
			for (int k=0;k<t.side.num;k++){
				vertex[t.side[k].vertex].is_selected = true;
				edge[t.side[k].edge].is_selected = true;
			}
	notify(MESSAGE_SELECTION);
}

void DataModel::selectionFromEdges()
{
	for (ModelVertex &v: vertex)
		v.is_selected = false;
	for (ModelEdge &e: edge)
		if (e.is_selected)
			for (int k=0;k<2;k++)
				vertex[e.vertex[k]].is_selected = true;
	for (ModelPolygon &p: polygon){
		p.is_selected = true;
		for (int k=0;k<p.side.num;k++)
			p.is_selected &= edge[p.side[k].edge].is_selected;
	}
	notify(MESSAGE_SELECTION);
}

void DataModel::selectionFromVertices() {
	for (ModelEdge &e: edge) {
		e.is_selected = (vertex[e.vertex[0]].is_selected and vertex[e.vertex[1]].is_selected);
		e.view_stage = min(vertex[e.vertex[0]].view_stage, vertex[e.vertex[1]].view_stage);
	}
	for (ModelPolygon &t: polygon) {
		t.is_selected = true;
		t.view_stage = vertex[t.side[0].vertex].view_stage;
		for (int k=0;k<t.side.num;k++) {
			t.is_selected &= vertex[t.side[k].vertex].is_selected;
			t.view_stage = min(t.view_stage, vertex[t.side[k].vertex].view_stage);
		}
	}
	notify(MESSAGE_SELECTION);
}

ModelPolygon *DataModel::addTriangle(int a, int b, int c, int material) {
	return (ModelPolygon*)execute(new ActionModelAddPolygonSingleTexture({a,b,c}, material, {vector::EY, v_0, vector::EX}));
}

ModelPolygon *DataModel::addPolygon(const Array<int> &v, int material)
{
	Array<vector> sv;
	for (int i=0;i<v.num;i++){
		float w = (float)i / (float)v.num * 2 * pi;
		sv.add(vector(0.5f + cos(w) * 0.5f, 0.5f + sin(w), 0));
	}
	return (ModelPolygon*)execute(new ActionModelAddPolygonSingleTexture(v, material, sv));
}

ModelPolygon *DataModel::addPolygonWithSkin(const Array<int> &v, const Array<vector> &sv, int material)
{
	return (ModelPolygon*)execute(new ActionModelAddPolygonSingleTexture(v, material, sv));
}



void DataModel::createSkin(ModelSkin *src, ModelSkin *dst, float quality_factor)
{
	msg_todo("DataModel::CreateSkin");
}


bool int_array_has_duplicates(const Array<int> &a) {
	for (int i=0; i<a.num; i++)
		for (int j=i+1; j<a.num; j++)
			if (a[i] == a[j])
				return true;
	return false;
}


void DataModel::_addPolygon(const Array<int> &v, int _material, const Array<vector> &sv, int index)
{
	if (int_array_has_duplicates(v))
		throw GeometryException("AddPolygon: duplicate vertices");

	ModelPolygon t;
	t.side.resize(v.num);
	for (int k=0;k<v.num;k++){
		t.side[k].vertex = v[k];
		for (int i=0;i<material[_material]->texture_levels.num;i++)
			t.side[k].skin_vertex[i] = sv[i * v.num + k];
	}
	for (int k=0;k<v.num;k++){
		try{
			t.side[k].edge = add_edge_for_new_polygon(t.side[k].vertex, t.side[(k + 1) % v.num].vertex, polygon.num, k);
			t.side[k].edge_direction = edge[t.side[k].edge].ref_count - 1;
		}catch(GeometryException &e){
			// failed -> clean up
			for (int i=edge.num-1;i>=0;i--)
				for (int j=0;j<edge[i].ref_count;j++)
					if (edge[i].polygon[j] == polygon.num){
						edge[i].ref_count --;
						if (edge[i].ref_count == 0)
							edge.resize(i);
					}
			throw(e);
		}
	}

	for (int vv: v)
		vertex[vv].ref_count ++;

	// closed?
//	updateClosed();

	t.is_selected = false;
	t.material = _material;
	t.view_stage = ed->multi_view_3d->view_stage;
	t.normal_dirty = true;
	t.triangulation_dirty = true;
	if (index >= 0){
		polygon.insert(t, index);

		// correct edges
		for (ModelEdge &e: edge)
			for (int k=0;k<e.ref_count;k++)
				if (e.polygon[k] >= index)
					e.polygon[k] ++;

		// correct own edges
		for (int k=0;k<t.side.num;k++)
			edge[polygon[index].side[k].edge].polygon[polygon[index].side[k].edge_direction] = index;
	}else
		polygon.add(t);
}

void DataModel::_removePolygon(int index)
{
	ModelPolygon &t = polygon[index];

	// unref the vertices
	for (int k=0;k<t.side.num;k++){
		vertex[t.side[k].vertex].ref_count --;
	}

	Set<int> obsolete;

	// remove from its edges
	for (int k=0;k<t.side.num;k++){
		ModelEdge &e = edge[t.side[k].edge];
		e.ref_count --;
		if (e.ref_count > 0){
			// edge has other triangle...
			if (t.side[k].edge_direction > 0){
				e.polygon[1] = -1;
			}else{
				// flip ownership
				e.polygon[0] = e.polygon[1];
				e.side[0] = e.side[1];
				e.polygon[1] = -1;

				// swap vertices
				int v = e.vertex[0];
				e.vertex[0] = e.vertex[1];
				e.vertex[1] = v;

				// relink other triangle
				polygon[e.polygon[0]].side[e.side[0]].edge_direction = 0;
			}
		}else{
			e.polygon[0] = -1;
			obsolete.add(t.side[k].edge);
		}
	}

	// correct edge links
	foreachi(ModelEdge &e, edge, i)
		for (int k=0;k<e.ref_count;k++)
			if (e.polygon[k] > index)
				e.polygon[k] --;
			else if (e.polygon[k] == index){
				throw GeometryException("RemoveTriangle: tria == index");
			}

	polygon.erase(index);

	//TestSanity("rem poly 0");

	// remove obsolete edges
	foreachb(int o, obsolete)
		remove_obsolete_edge(o);

/*	if (!TestSanity("rem poly"))
		throw GeometryException("RemoveTriangle: TestSanity failed");*/
}

void DataModel::build_topology()
{
	// clear
	edge.clear();
	for (ModelVertex &v: vertex)
		v.ref_count = 0;

	// add all triangles
	foreachi(ModelPolygon &t, polygon, ti){
		// vertices
		for (int k=0;k<t.side.num;k++)
			vertex[t.side[k].vertex].ref_count ++;

		// edges
		for (int k=0;k<t.side.num;k++){
			t.side[k].edge = add_edge_for_new_polygon(t.side[k].vertex, t.side[(k + 1) % t.side.num].vertex, ti, k);
			t.side[k].edge_direction = edge[t.side[k].edge].ref_count - 1;
		}
	}

//	updateClosed();
}


void DataModel::remove_obsolete_edge(int index)
{
	// correct triangle references
	for (ModelPolygon &t: polygon)
		for (int k=0;k<t.side.num;k++)
			if (t.side[k].edge > index)
				t.side[k].edge --;
			else if (t.side[k].edge == index)
				msg_error(format("surf rm edge: edge not really obsolete  rc=%d (%d,%d) (%d,%d)", edge[index].ref_count, t.side[k].vertex, t.side[(k+1)%t.side.num].vertex, edge[index].vertex[0], edge[index].vertex[1]));

	// delete
	edge.erase(index);
}

int DataModel::add_edge_for_new_polygon(int a, int b, int tria, int side)
{
	foreachi(ModelEdge &e, edge, i){
		if ((e.vertex[0] == a) && (e.vertex[1] == b)){
			throw GeometryException("the new polygon would have neighbors of opposite orientation");
			/*e.RefCount ++;
			msg_error("surface error? inverse edge");
			e.Polygon[1] = tria;
			e.Side[1] = side;
			return i;*/
		}
		if ((e.vertex[0] == b) && (e.vertex[1] == a)){
			if (e.polygon[0] == tria)
				throw GeometryException("the new polygon would contain the same edge twice");
			if (e.ref_count > 1)
				throw GeometryException("there would be more than 2 polygons sharing an egde");
			e.ref_count ++;
			e.polygon[1] = tria;
			e.side[1] = side;
			return i;
		}
	}
	ModelEdge ee;
	ee.vertex[0] = a;
	ee.vertex[1] = b;
	ee.is_selected = false;
	ee.is_special = false;
	ee.is_round = false;
	ee.ref_count = 1;
	ee.polygon[0] = tria;
	ee.side[0] = side;
	ee.polygon[1] = -1;
	edge.add(ee);
	return edge.num - 1;
}

float DataModel::getRadius()
{
	float radius = 0;
	for (ModelVertex &v: vertex)
		radius = max(v.pos.length(), radius);
	return radius;
}

float DetailDistTemp1,DetailDistTemp2,DetailDistTemp3;

int get_num_trias(DataModel *m, ModelSkin *s)
{
	int n = 0;
	for (int i=0;i<m->material.num;i++)
		n += s->sub[i].triangle.num;
	return n;
}

void DataModel::generateDetailDists(float *dist)
{
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
	vector min = v_0, max = v_0;
	for (ModelVertex &v: vertex){
		min._min(v.pos);
		max._max(v.pos);
	}
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

	begin_inside_tests();

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
				if (inside_test(r)){
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


	end_inside_tests();

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

struct SurfaceInsideTestData : BspTree{};
static SurfaceInsideTestData *inside_data = NULL;

void DataModel::begin_inside_tests()
{
#if 0
	if (!is_closed)
		return;
	inside_data = new SurfaceInsideTestData;
	/*inside_data->num_trias = 0;
	for (ModelPolygon &t, Polygon)
		inside_data->num_trias += (t.Side.num - 2);
	inside_data->ray.resize(inside_data->num_trias * 3);
	inside_data->pl.resize(inside_data->num_trias);
	Ray *r = &inside_data->ray[0];
	plane *pl = &inside_data->pl[0];
	for (ModelPolygon &t, Polygon){
		if (t.TriangulationDirty)
			t.UpdateTriangulation(model->Vertex);
		for (int k=0;k<t.Side.num-2;k++){
			*(pl ++) = plane(model->Vertex[t.Side[0].Vertex].pos, t.TempNormal);
			*(r ++) = Ray(model->Vertex[t.Side[t.Side[k].Triangulation[0]].Vertex].pos, model->Vertex[t.Side[t.Side[k].Triangulation[1]].Vertex].pos);
			*(r ++) = Ray(model->Vertex[t.Side[t.Side[k].Triangulation[1]].Vertex].pos, model->Vertex[t.Side[t.Side[k].Triangulation[2]].Vertex].pos);
			*(r ++) = Ray(model->Vertex[t.Side[t.Side[k].Triangulation[2]].Vertex].pos, model->Vertex[t.Side[t.Side[k].Triangulation[0]].Vertex].pos);
		}
	}*/

	float epsilon = getRadius() * 0.001f;
	for (ModelPolygon &p: polygon)
		inside_data->add(p, this, epsilon);
#endif
}

bool DataModel::inside_test(const vector &p)
{
	if (!inside_data)
		return false;
	return inside_data->inside(p);

}

void DataModel::end_inside_tests()
{
//	if (inside_data)
	//	delete(inside_data);
}

int DataModel::getNumSelectedVertices()
{
	int r = 0;
	/*if ((CreationMode < 0) and ((SubMode == SubModeSkeleton) || ((SubMode == SubModeAnimation) and (move->Type == MoveTypeSkeletal)))){
		for (int i=0;i<Bone.num;i++)
			if (Bone[i].IsSelected)
				r++;
		return r;
	}*/
	for (ModelVertex &v: vertex)
		if (v.is_selected)
			r ++;
	return r;
}

int DataModel::getNumSelectedSkinVertices()
{
	int r = 0;
	for (ModelSkinVertexDummy &v: skin_vertex)
		if (v.is_selected)
			r ++;
	return r;
}

int DataModel::getNumSelectedPolygons()
{
	int r = 0;
	for (ModelPolygon &t: polygon)
		if (t.is_selected)
			r ++;
	return r;
}

int DataModel::getNumSelectedEdges()
{
	int r = 0;
	for (ModelEdge &e: edge)
		if (e.is_selected)
			r ++;
	return r;
}

int DataModel::getNumSelectedBones()
{
	int r = 0;
	for (ModelBone &b: bone)
		if (b.is_selected)
			r ++;
	return r;
}

int DataModel::getNumPolygons()
{
	return polygon.num;
}

void DataModel::reconnectBone(int index, int parent)
{	execute(new ActionModelReconnectBone(index, parent));	}

void DataModel::setBoneModel(int index, const string &filename)
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

void DataModel::copyGeometry(Geometry &geo)
{
	geo.clear();

	// copy vertices
	Array<int> vert;
	foreachi(ModelVertex &v, vertex, vi)
		if (v.is_selected){
			geo.vertex.add(v);
			vert.add(vi);
		}

	// copy triangles
	for (ModelPolygon &t: polygon)
		if (t.is_selected){
			ModelPolygon tt = t;
			for (int k=0;k<t.side.num;k++)
				foreachi(int v, vert, vi)
					if (v == t.side[k].vertex)
						tt.side[k].vertex = vi;
			geo.polygon.add(tt);
		}
}

void DataModel::delete_selection(const ModelSelectionState &s, bool greedy)
{	execute(new ActionModelDeleteSelection(s, greedy));	}

void DataModel::delete_polygon(int index)
{	execute(new ActionModelSurfaceDeletePolygon(index));	}

void DataModel::invert_polygons(const ModelSelectionState &s) {
	execute(new ActionModelSurfaceInvert(s.polygon, selection_consistent_surfaces(s, this)));
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

void DataModel::subdivideSelectedSurfaces(const ModelSelectionState &s)
{
	//execute(new ActionModelSurfacesSubdivide(getSelectedSurfaces()));
	msg_todo("subdivide");
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


void ModelSelectionState::clear() {
	vertex.clear();
	polygon.clear();
	edge.clear();
}

bool selection_consistent_surfaces(const ModelSelectionState &s, DataModel *m) {
	for (int ei: s.edge) {
		auto &e = m->edge[ei];
		if (!s.polygon.contains(e.polygon[0]))
			return false;
		if (!s.polygon.contains(e.polygon[1]))
			return false;
	}
	return true;
}

void ModelSelectionState::expand_to_surfaces(DataModel *m) {
	while (true) {
		bool changed = false;
		for (auto &e: m->edge)
			if (vertex.contains(e.vertex[0]) != vertex.contains(e.vertex[1])) {
				vertex.add(e.vertex[0]);
				vertex.add(e.vertex[1]);
				changed = true;
			}
		if (!changed)
			break;
	}
	foreachi (auto &p, m->polygon, i)
		for (int k=0; k<p.side.num; k++)
			if (vertex.contains(p.side[k].vertex))
				polygon.add(i);
}

ModelSelectionState DataModel::get_selection() const {
	ModelSelectionState s;
	foreachi(ModelVertex &v, vertex, i)
		if (v.is_selected)
			s.vertex.add(i);
	foreachi(ModelPolygon &t, polygon, j)
		if (t.is_selected)
			s.polygon.add(j);
	foreachi(ModelEdge &e, edge, j)
		if (e.is_selected)
			s.edge.add(j);
	return s;
}

void DataModel::set_selection(const ModelSelectionState& s) {
	clearSelection();
	for (int v: s.vertex)
		vertex[v].is_selected = true;
	for (int p: s.polygon)
		polygon[p].is_selected = true;
	for (int e: s.edge)
		edge[e].is_selected = true;
	notify(MESSAGE_SELECTION);
}


Array<int> DataModel::get_boundary_loop(int v0)
{
	Array<int> loop;
	int last = v0;
	bool found = true;
	while(found){
		found = false;
		for (ModelEdge &e: edge)
			if (e.ref_count == 1)
				if (e.vertex[0] == last){
					last = e.vertex[1];
					loop.add(last);
					if (last == v0)
						return loop;
					found = true;
					break;
				}
	}
	return loop;
}



/*int DataModel::GetNumMarkedBalls()
{
	int r=0;
	for (int i=0;i<Ball.num;i++)
		if (Ball[i].IsSelected)
			r++;
	return r;
}

int DataModel::GetNumMarkedKonvPolys()
{
	int r=0;
	for (int i=0;i<Poly.num;i++)
		if (Poly[i].IsSelected)
			r++;
	return r;
 }*/

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
