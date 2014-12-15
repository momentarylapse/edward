/*
 * LightmapData.cpp
 *
 *  Created on: 18.05.2013
 *      Author: michi
 */

#include "LightmapData.h"
#include "../../../Data/World/DataWorld.h"
#include "../../../Data/Model/DataModel.h"
#include "../../../x/object.h"
#include "../../../x/model_manager.h"
#include "../../../x/terrain.h"

bool LightmapData::Triangle::intersect(const Ray &r, vector &cp) const
{
	bool r0 = (r.dot(ray[0]) > 0);
	bool r1 = (r.dot(ray[1]) > 0);
	if (r1 != r0)
		return false;
	bool r2 = (r.dot(ray[2]) > 0);
	if (r2 != r0)
		return false;

	return r.intersect_plane(pl, cp);
}

LightmapData::LightmapData(DataWorld *w)
{
	emissive_brightness = 1.0f;
	allow_sun = true;
	replace_objects = false;

	color_exponent = 0.7f;

	Init(w);
}

LightmapData::~LightmapData()
{
	foreach(Model &m, Models)
		delete(m.orig);
}

void LightmapData::Init(DataWorld *w)
{
	msg_db_f("LightmapData.Init", 1);
	source_world = w;
	world_name_small = w->filename.basename().replace(".world", "");

	area = 0;

	// load data
	foreachi(WorldObject &o, w->Objects, i)
		if ((o.is_selected) && (!o.object->active_physics)){
			o.object->UpdateMatrix();
			AddModel(o.FileName, o.object->_matrix, i);
		}
	foreachi(WorldTerrain &t, w->Terrains, i)
		if (t.is_selected)
			AddTerrain(t, i);
	if (Trias.num == 0)
		return;

	min =  max = Trias[0].v[0];
	for (int i=0;i<Trias.num;i++)
		for (int k=0;k<3;k++){
			min._min(Trias[i].v[k]);
			max._max(Trias[i].v[k]);
		}
	center = (min + max) / 2;
	large_distance = 100 * (max - min).length();

	/*for (int i=0;i<vertex.num;i++)
		vertex[i].v -= lm_m;
	for (int i=0;i<tria.num;i++){
		tria[i].v[0] -= lm_m;
		tria[i].v[1] -= lm_m;
		tria[i].v[2] -= lm_m;
		tria[i].m -= lm_m;
	}
	for (int i=0;i<light.num;i++)
		if (light[i].Directional)
			light[i].Pos -= lm_m;*/

	if (w->meta_data.SunEnabled){
		Light l;
		l.Directional = true;
		l.Dir = -w->meta_data.SunAng.ang2dir();
		l.Ambient = w->meta_data.SunAmbient;
		l.Diffuse = w->meta_data.SunDiffuse;
		l.Specular = Black;
		Lights.add(l);
	}
	Ambient = w->meta_data.Ambient;

	SetResolution(GuessResolution());
}

float LightmapData::GuessResolution()
{
	float area_max = 0;
	foreach(Model &m, Models)
		if (m.area > area_max)
			area_max = m.area;
	/*foreach(Terrain &t, Terrains)
		if (t.area > area_max)
			area_max = t.area;*/
	return sqrt(area_max) / 512;
}

void LightmapData::SetResolution(float res)
{
	resolution = res;
	foreach(Model &m, Models){
		m.tex_width = min(1 << (int)(log(sqrt(m.area) / resolution)/log(2.0f) - 0.5f), 1024);
		m.tex_height = m.tex_width;
	}
	foreach(Terrain &t, Terrains){
		t.tex_width = min(1 << (int)(log(sqrt(t.area) / resolution)/log(2.0f) - 0.5f), 1024);
		t.tex_height = t.tex_width;
	}
}


static void update_tria(LightmapData::Triangle &t)
{
	PlaneFromPoints(t.pl, t.v[0], t.v[1], t.v[2]);
	t.ray[0] = Ray(t.v[0], t.v[1]);
	t.ray[1] = Ray(t.v[1], t.v[2]);
	t.ray[2] = Ray(t.v[2], t.v[0]);
	t.area = ((t.v[1] - t.v[0]) ^ (t.v[2] - t.v[0])).length() / 2;
}

static void tria_set_mat(LightmapData::Triangle &t, Material *m)
{
	t.am = m->ambient;
	t.di = m->diffuse;
	t.em = m->emission;
}

static void tria_set_mat(LightmapData::Triangle &t, ModelMaterial *m)
{
	t.am = m->ambient;
	t.di = m->diffuse;
	t.em = m->emission;
}

void LightmapData::AddModel(const string &filename, matrix &mat, int object_index)
{
	msg_db_f("lm_add_model", 1);

	Model mod;
	mod.mat = mat;
	mod.id = Models.num;

	mod.orig_name = filename.basename();
	msg_write(mod.orig_name);
	mod.offset = Trias.num;
	mod.object_index = object_index;
	mod.area = 0;

	DataModel *m = new DataModel();
	mod.orig = m;
	m->load(ObjectDir + filename + ".model");


	mod.new_name = format("Lightmap/%s/%s_%d", world_name_small.c_str(), mod.orig_name.c_str(), mod.id);

	foreachi(ModelSurface &s, m->surface, surf){
		s.UpdateNormals();
		foreachi(ModelPolygon &p, s.polygon, i){
			if (p.triangulation_dirty)
				p.UpdateTriangulation(m->vertex);
			for (int k=0;k<p.side.num-2;k++){
				Triangle t;
				t.mod_id = mod.id;
				t.ter_id = -1;
				t.surf = surf;
				t.poly = i;
				t.side = k;
				for (int l=0;l<3;l++){
					int n = p.side[k].triangulation[l];
					t.v[l] = mat * m->vertex[p.side[n].vertex].pos;
					t.n[l] = mat.transform_normal(p.side[n].normal);
				}
				update_tria(t);
				tria_set_mat(t, &m->material[p.material]);
				mod.area += t.area;
				Trias.add(t);
			}
		}
	}
	area += mod.area;


	// lights
	foreach(ModelEffect &fx, m->fx){
		if (fx.kind == FXTypeLight){
			msg_write("......fx light");
			Light l;
			l.Directional = false;
			l.Pos = mat * m->vertex[fx.vertex].pos;
			l.Radius = fx.size;
			l.Ambient = fx.colors[0];
			l.Diffuse = fx.colors[1];
			l.Specular = fx.colors[2];
			Lights.add(l);
		}
	}


	mod.num_trias = Trias.num - mod.offset;
	Models.add(mod);
}


void LightmapData::AddTerrain(WorldTerrain &wt, int terrain_index)
{
	msg_db_f("lm_add_terrain", 1);

	Terrain ter;
	ter.id = Terrains.num;

	ter.orig_name = wt.FileName.basename();
	msg_write(ter.orig_name);
	ter.offset = Trias.num;
	ter.terrain_index = terrain_index;
	ter.area = 0;

	ter.orig = wt.terrain;
	::Terrain *tt = wt.terrain;

	ter.new_name = format("Lightmap/%s/%s_%d", world_name_small.c_str(), ter.orig_name.c_str(), ter.id);

	for (int x=0;x<tt->num_x;x++)
		for (int z=0;z<tt->num_z;z++){
			int a = x*(tt->num_z+1)+z;
			int b = x*(tt->num_z+1)+z+1;
			int c = (x+1)*(tt->num_z+1)+z;
			int d = (x+1)*(tt->num_z+1)+z+1;
			Triangle t;
			t.mod_id = -1;
			t.ter_id = ter.id;
			t.poly = a * 2;
			t.v[0] = tt->vertex[a];
			t.v[1] = tt->vertex[b];
			t.v[2] = tt->vertex[d];
			t.n[0] = tt->normal[a];
			t.n[1] = tt->normal[b];
			t.n[2] = tt->normal[d];
			update_tria(t);
			tria_set_mat(t, tt->material);
			ter.area += t.area;
			Trias.add(t);
			t.poly = a * 2 + 1;
			t.v[0] = tt->vertex[a];
			t.v[1] = tt->vertex[d];
			t.v[2] = tt->vertex[c];
			t.n[0] = tt->normal[a];
			t.n[1] = tt->normal[d];
			t.n[2] = tt->normal[c];
			update_tria(t);
			tria_set_mat(t, tt->material);
			ter.area += t.area;
			Trias.add(t);
		}
	area += ter.area;


	ter.num_trias = Trias.num - ter.offset;
	Terrains.add(ter);
}

void LightmapData::AddTextureLevels(bool modify)
{
	foreach(Model &m, Models){
		if (modify){
			foreach(ModelMaterial &mat, m.orig->material)
				mat.num_textures ++;
		}
		m.orig->Automap(-1, 1); // TODO...
	}
}

rect get_tria_skin_boundary(vector sv[3])
{
	vector _min = sv[0];
	_min._min(sv[1]);
	_min._min(sv[2]);
	vector _max = sv[0];
	_max._max(sv[1]);
	_max._max(sv[2]);
	return rect(_min.x, _max.x, _min.y, _max.y);
}

void LightmapData::Triangle::Rasterize(LightmapData *l, int i)
{
	// rasterize triangle
	rect r = get_tria_skin_boundary(sv);
	int v_offset = l->Vertices.num;
	for (int x=r.x1-1;x<r.x2+1;x++)
		for (int y=r.y1-1;y<r.y2+1;y++){
			vector c = vector((float)x + 0.5f, (float)y + 0.5f, 0);
			float f, g;
			GetBaryCentric(c, sv[0], sv[1], sv[2], f, g);
			if ((f >= 0) && (g >= 0) && (f + g <= 1)){
				Vertex vv;
				vv.pos = v[0] + f * (v[1] - v[0]) + g * (v[2] - v[0]);
				vv.n = n[0] + f * (n[1] - n[0]) + g * (n[2] - n[0]);
				vv.x = x;
				vv.y = y;
				vv.tria_id = i;
				vv.mod_id = mod_id;
				vv.ter_id = ter_id;
				vv.am = am;
				vv.dif = di;
				vv.em = em;
				l->Vertices.add(vv);
			}
		}
	num_vertices = l->Vertices.num - v_offset;

	// guess vertex areas
	for (int j=v_offset;j<l->Vertices.num;j++)
		l->Vertices[j].area = area / num_vertices;
}

void LightmapData::CreateVertices()
{
	Vertices.clear();
	foreach(Model &m, Models){
		int w = m.tex_width;
		int h = m.tex_height;

		for (int i=m.offset;i<m.offset + m.num_trias;i++){
			LightmapData::Triangle &t = Trias[i];
			ModelPolygon &p = m.orig->surface[t.surf].polygon[t.poly];
			for (int k=0;k<3;k++){
				int si = p.side[t.side].triangulation[k];
				t.sv[k] = p.side[si].skin_vertex[1];
				t.sv[k].x *= w;
				t.sv[k].y *= h;
			}

			t.Rasterize(this, i);
		}
	}
	foreach(Terrain &ter, Terrains){
		int w = ter.tex_width;
		int h = ter.tex_height;

		for (int i=ter.offset;i<ter.offset + ter.num_trias;i++){
			LightmapData::Triangle &t = Trias[i];
			for (int k=0;k<3;k++){
				t.sv[k].x = t.v[k].x - ter.orig->pos.x;
				t.sv[k].y = t.v[k].z - ter.orig->pos.z;
				t.sv[k].z = 0;
				t.sv[k].x *= (float)w / (ter.orig->pattern.x * ter.orig->num_x);
				t.sv[k].y *= (float)h / (ter.orig->pattern.z * ter.orig->num_z);
			}

			t.Rasterize(this, i);
		}
	}
	msg_write("Vertices: " + i2s(Vertices.num));
}

bool LightmapData::IsVisible(const vector &a, const vector &b, int ignore_tria1, int ignore_tria2)
{
	Ray r = Ray(a, b);

	for (int ti=0;ti<Trias.num;ti++){
		if ((ti == ignore_tria1) || (ti == ignore_tria2))
			continue;
		LightmapData::Triangle &t = Trias[ti];
		vector cp;
/*		if (VecLineDistance(tria[t].m, p, p2) > tria[t].r)
		//if (_vec_line_distance_(tria[t].m, p1, p2) > tria[t].r)
			continue;*/
		if (!t.intersect(r, cp))
			continue;

		if (_vec_between_(cp, a, b))
			return false;
	}
	return true;
}

bool LightmapData::IsVisible(Vertex &a, Vertex &b)
{
	if (a.tria_id == b.tria_id)
		return false;
	vector dir = b.pos - a.pos;
	if (a.n * dir < 0)
		return false;
	if (b.n * dir > 0)
		return false;

	return IsVisible(a.pos, b.pos, a.tria_id, b.tria_id);
}

