/*
 * DataModel.cpp
 *
 *  Created on: 04.03.2012
 *      Author: michi
 */

#include "DataModel.h"
#include "ModelSurface.h"
#include "Geometry/Geometry.h"
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
	surface = -1;
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
	surface.clear();
	vertex.clear();
	ball.clear();
	cylinder.clear();
	poly.clear();
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
	msg_write(surface.num);
	for (ModelSurface &s: surface){
		msg_write(s.polygon.num);
		s.testSanity("Model.DebugShow");
	}
}

bool DataModel::test_sanity(const string &loc)
{
	for (ModelSurface &s: surface){
		if (!s.testSanity(loc))
			return false;
	}
	return true;
}


void DataModel::on_post_action_update()
{
	showVertices(vertex);

	updateNormals();
	for (ModelSurface &s: surface){
		s.pos = v_0;
		for (int k=0;k<s.vertex.num;k++)
			s.pos += vertex[s.vertex[k]].pos;
		s.pos /= s.vertex.num;
		for (ModelPolygon &p: s.polygon){
			p.pos = v_0;
			for (int k=0;k<p.side.num;k++)
				p.pos += vertex[p.side[k].vertex].pos;
			p.pos /= p.side.num;
		}
		for (ModelEdge &e: s.edge)
			e.pos = (vertex[e.vertex[0]].pos + vertex[e.vertex[1]].pos) / 2;
	}
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
	surface.clear();

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
	for (ModelPolyhedron &p: poly){
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
		surface.back().is_physical = true;
		surface.back().is_visible = false;
	}
	poly.clear();

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
	for (ModelSurface &s: surface){
		if (!s.is_visible)
			continue;
		for (ModelPolygon &t: s.polygon){
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

	for (ModelSurface &s: surface)
		for (ModelPolygon &t: s.polygon)
			for (int k=0;k<t.side.num;k++)
				if (!t.normal_dirty)
					if (sindex.contains(t.side[k].vertex)){
						t.normal_dirty = true;
						break;
					}
}

void DataModel::setAllNormalsDirty()
{
	for (ModelSurface &s: surface)
		for (ModelPolygon &t: s.polygon)
			t.normal_dirty = true;
}

void DataModel::updateNormals()
{
	for (ModelSurface &s: surface)
		s.updateNormals();
}

ModelSurface *DataModel::addSurface(int surf_no)
{
	ModelSurface s;
	s.model = this;
	s.view_stage = ed->multi_view_3d->view_stage;
	s.is_selected = true;
	s.is_closed = false;
	s.is_visible = true;
	s.is_physical = true;
	if (surf_no >= 0){
		surface.insert(s, surf_no);
		return &surface[surf_no];
	}else{
		surface.add(s);
		return &surface.back();
	}
}


void DataModel::addVertex(const vector &pos, int bone_index, int normal_mode)
{	execute(new ActionModelAddVertex(pos, bone_index, normal_mode));	}

void DataModel::clearSelection()
{
	for (ModelVertex &v: vertex)
		v.is_selected = false;
	for (ModelSurface &s: surface){
		s.is_selected = false;
		for (ModelPolygon &t: s.polygon)
			t.is_selected = false;
		for (ModelEdge &e: s.edge)
			e.is_selected = false;
	}
	notify(MESSAGE_SELECTION);
}

void DataModel::selectionFromSurfaces()
{
	for (ModelVertex &v: vertex)
		v.is_selected = false;
	for (ModelSurface &s: surface){
		for (int v: s.vertex)
			vertex[v].is_selected = s.is_selected;
		for (ModelPolygon &t: s.polygon)
			t.is_selected = s.is_selected;
		for (ModelEdge &e: s.edge)
			e.is_selected = s.is_selected;
	}
	notify(MESSAGE_SELECTION);
}

void DataModel::selectionFromPolygons()
{
	for (ModelVertex &v: vertex)
		v.is_selected = false;
	for (ModelSurface &s: surface){
		for (ModelEdge &e: s.edge)
			e.is_selected = false;
		for (ModelPolygon &t: s.polygon)
			if (t.is_selected)
				for (int k=0;k<t.side.num;k++){
					vertex[t.side[k].vertex].is_selected = true;
					s.edge[t.side[k].edge].is_selected = true;
				}
	}
	for (ModelSurface &s: surface){
		s.is_selected = true;
		for (ModelPolygon &t: s.polygon)
			s.is_selected &= t.is_selected;
	}
	notify(MESSAGE_SELECTION);
}

void DataModel::selectionFromEdges()
{
	for (ModelVertex &v: vertex)
		v.is_selected = false;
	for (ModelSurface &s: surface){
		for (ModelEdge &e: s.edge)
			if (e.is_selected)
				for (int k=0;k<2;k++)
					vertex[e.vertex[k]].is_selected = true;
		for (ModelPolygon &p: s.polygon){
			p.is_selected = true;
			for (int k=0;k<p.side.num;k++)
				p.is_selected &= s.edge[p.side[k].edge].is_selected;
		}
		s.is_selected = true;
		for (ModelEdge &e: s.edge)
			s.is_selected &= e.is_selected;
	}
	notify(MESSAGE_SELECTION);
}

void DataModel::selectionFromVertices()
{
	for (ModelSurface &s: surface){
		s.is_selected = true;
		for (ModelEdge &e: s.edge){
			e.is_selected = (vertex[e.vertex[0]].is_selected and vertex[e.vertex[1]].is_selected);
			e.view_stage = min(vertex[e.vertex[0]].view_stage, vertex[e.vertex[1]].view_stage);
		}
		for (ModelPolygon &t: s.polygon){
			t.is_selected = true;
			t.view_stage = vertex[t.side[0].vertex].view_stage;
			for (int k=0;k<t.side.num;k++){
				t.is_selected &= vertex[t.side[k].vertex].is_selected;
				t.view_stage = min(t.view_stage, vertex[t.side[k].vertex].view_stage);
			}
			s.is_selected &= t.is_selected;
		}
	}
	notify(MESSAGE_SELECTION);
}

void DataModel::selectOnlySurface(ModelSurface *s)
{
	for (ModelSurface &ss: surface)
		ss.is_selected = (&ss == s);
	selectionFromSurfaces();
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



int DataModel::get_surf_no(ModelSurface *s)
{
	foreachi(ModelSurface &ss, surface, i)
		if (&ss == s)
			return i;
	return -1;
}

ModelSurface *DataModel::surfaceJoin(ModelSurface *a, ModelSurface *b)
{
	a->testSanity("Join prae a");
	b->testSanity("Join prae b");

	int ai = get_surf_no(a);
	int bi = get_surf_no(b);

	// correct edge data of b
	for (ModelEdge &e: b->edge){
		if (e.polygon[0] >= 0)
			e.polygon[0] += a->polygon.num;
		if (e.polygon[1] >= 0)
			e.polygon[1] += a->polygon.num;
	}

	// correct triangle data of b
	for (ModelPolygon &t: b->polygon)
		for (int k=0;k<t.side.num;k++)
			t.side[k].edge += a->edge.num;

	// correct vertex data of b
	for (int v: b->vertex)
		vertex[v].surface = ai;

	// insert data
	a->vertex.join(b->vertex);
	a->edge.append(b->edge);
	a->polygon.append(b->polygon);

	// remove surface
	if (bi >= 0)
		surface.erase(bi);
	a = &surface[ai];
	a->testSanity("Join post a");

	return a;
}

void DataModel::createSkin(ModelSkin *src, ModelSkin *dst, float quality_factor)
{
	msg_todo("DataModel::CreateSkin");
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

	for (ModelSurface &s: surface)
		s.beginInsideTests();

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
				for (ModelSurface &s: surface)
					if (s.insideTest(r)){
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


	for (ModelSurface &s: surface)
		s.endInsideTests();

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
	for (ModelSurface &s: surface)
		for (ModelPolygon &t: s.polygon)
			if (t.is_selected)
				r ++;
	return r;
}

int DataModel::getNumSelectedEdges()
{
	int r = 0;
	for (ModelSurface &s: surface)
		for (ModelEdge &e: s.edge)
			if (e.is_selected)
				r ++;
	return r;
}

int DataModel::getNumSelectedSurfaces()
{
	int r = 0;
	for (ModelSurface &s: surface)
		if (s.is_selected)
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
	int r = 0;
	for (ModelSurface &s: surface)
		r += s.polygon.num;
	return r;
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
	for (ModelSurface &s: surface)
		for (ModelPolygon &t: s.polygon)
			if (t.is_selected){
				ModelPolygon tt = t;
				for (int k=0;k<t.side.num;k++)
					foreachi(int v, vert, vi)
						if (v == t.side[k].vertex)
							tt.side[k].vertex = vi;
				geo.polygon.add(tt);
			}
}

void DataModel::deleteSelection(bool greedy)
{	execute(new ActionModelDeleteSelection(greedy));	}

void DataModel::invertSurfaces(const Set<int> &surfaces)
{	execute(new ActionModelSurfaceInvert(surfaces));	}

void DataModel::invertSelection()
{	invertSurfaces(getSelectedSurfaces());	}

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

void DataModel::subdivideSelectedSurfaces()
{	execute(new ActionModelSurfacesSubdivide(getSelectedSurfaces()));	}

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


void ModelSelectionState::clear()
{
	vertex.clear();
	surface.clear();
	polygon.clear();
}

Set<int> DataModel::getSelectedVertices()
{
	Set<int> vv;
	foreachi(ModelVertex &v, vertex, i)
		if (v.is_selected)
			vv.add(i);
	return vv;
}

Set<int> DataModel::getSelectedSurfaces()
{
	Set<int> ss;
	foreachi(ModelSurface &surf, surface, i)
		if (surf.is_selected)
			ss.add(i);
	return ss;
}

void DataModel::getSelectionState(ModelSelectionState& s)
{
	s.clear();
	foreachi(ModelVertex &v, vertex, i)
		if (v.is_selected)
			s.vertex.add(i);
	foreachi(ModelSurface &surf, surface, i){
		if (surf.is_selected)
			s.surface.add(i);
		Set<int> sel;
		foreachi(ModelPolygon &t, surf.polygon, j)
			if (t.is_selected)
				sel.add(j);
		s.polygon.add(sel);
		Array<ModelSelectionState::EdgeSelection> esel;
		foreachi(ModelEdge &e, surf.edge, j)
			if (e.is_selected)
				esel.add(e.vertex);
		s.edge.add(esel);
	}
}

void DataModel::setSelectionState(ModelSelectionState& s)
{
	clearSelection();
	for (int v: s.vertex)
		vertex[v].is_selected = true;
	for (int si: s.surface)
		surface[si].is_selected = true;
	for (int i=0;i<s.polygon.num;i++)
		for (int j: s.polygon[i])
			surface[i].polygon[j].is_selected = true;
	for (int i=0;i<s.edge.num;i++)
		for (ModelSelectionState::EdgeSelection &es: s.edge[i]){
			int ne = surface[i].findEdge(es.v[0], es.v[1]);
			if (ne >= 0)
				surface[i].edge[ne].is_selected = true;
		}
	notify(MESSAGE_SELECTION);
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

ModelSelectionState::EdgeSelection::EdgeSelection(int _v[2])
{
	v[0] = _v[0];
	v[1] = _v[1];
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
