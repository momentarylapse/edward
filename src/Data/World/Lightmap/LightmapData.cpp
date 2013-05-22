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

Ray::Ray(){}

Ray::Ray(const vector &a, const vector &b)
{
	u = b - a;
	v = b ^ a;
}

float Ray::dot(const Ray &r) const
{
	return (u * r.v) + (v * r.u);
}

bool Ray::intersect_plane(const plane &pl, vector &c) const
{
	float w = u * pl.n;
	if (w == 0)
		return false;
	c = ((v ^ pl.n) - u * pl.d) / w;
	return true;
}

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
	world_name_small = w->filename.basename().replace(".world", "");

	area = 0;

	// load data
	foreachi(WorldObject &o, w->Objects, i)
		if (o.is_selected){
			o.object->UpdateMatrix();
			msg_write(o.FileName);
			AddModel(o.FileName, o.object->_matrix, i);
		}
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
	//msg_write(min.str() + " " + max.str() + " " + f2s(large_distance, 3));

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
		l.Dir = w->meta_data.SunAng.ang2dir();
		l.Ambient = Black;//w->meta_data.SunColor;
		l.Diffuse = Gray;
		l.Specular = Black;
		Lights.add(l);
	}

	/*bool ok = true;
	for (int i=0;i<lmmodel.num;i++)
		if (!MapTriasToTexture(&lmmodel[i])){
			HuiErrorBox(LightmapDialog, _("Error"), _("Textur-Gr&o&se zu klein um alle Dreiecke aufzunehmen."));
			ok = false;
			break;
		}*/

	SetResolution(GuessResolution());
}

float LightmapData::GuessResolution()
{
	float area_max = 0;
	foreach(Model &m, Models)
		if (m.area > area_max)
			area_max = m.area;
	return sqrt(area_max) / 512;
}

void LightmapData::SetResolution(float res)
{
	resolution = res;
	foreach(Model &m, Models){
		m.tex_width = min(1 << (int)(log2(sqrt(m.area) / resolution) - 0.5f), 1024);
		m.tex_height = m.tex_width;
	}
}


void LightmapData::AddModel(const string &filename, matrix &mat, int object_index)
{
	msg_db_f("lm_add_model", 1);

	Model mod;
	mod.id = Models.num;

	mod.orig_name = filename.basename();
	msg_write(mod.orig_name);
	mod.offset = Trias.num;
	mod.object_index = object_index;
	mod.area = 0;

	DataModel *m = new DataModel();
	mod.orig = m;
	m->Load(ObjectDir + filename + ".model");


	mod.new_name = format("Lightmap/%s/%s_%d", world_name_small.c_str(), mod.orig_name.c_str(), mod.id);

	foreachi(ModelSurface &s, m->Surface, surf){
		s.UpdateNormals();
		foreachi(ModelPolygon &p, s.Polygon, i){
			if (p.TriangulationDirty)
				p.UpdateTriangulation(m);
			for (int k=0;k<p.Side.num-2;k++){
				Triangle t;
				t.mod_id = mod.id;
				t.surf = surf;
				t.poly = i;
				t.side = k;
				for (int l=0;l<3;l++){
					int n = p.Side[k].Triangulation[l];
					t.v[l] = mat * m->Vertex[p.Side[n].Vertex].pos;
					t.n[l] = mat.transform_normal(p.Side[0].Normal);
				}
				PlaneFromPoints(t.pl, t.v[0], t.v[1], t.v[2]);
				t.ray[0] = Ray(t.v[0], t.v[1]);
				t.ray[1] = Ray(t.v[1], t.v[2]);
				t.ray[2] = Ray(t.v[2], t.v[0]);
				t.em = m->Material[p.Material].Emission;
				t.area = ((t.v[1] - t.v[0]) ^ (t.v[2] - t.v[0])).length() / 2;
				mod.area += t.area;
				Trias.add(t);
			}
		}
	}
	area += mod.area;



		// lights
	/*	if (m->fx){
			msg_write("-fx-");
			sModelEffectData *fxd = m->fx_data;
			sEffect **_fx=(sEffect**)m->fx;
			while(*_fx){
				msg_write("......fx");
				if (fxd->type == FXTypeLight){
					msg_write("......light");
					sLightMapLight l;
					l.Directional = false;
					l.Pos = m->Skin[0]->Vertex[fxd->vertex];
					l.Radius = fxd->paramsf[0];
					l.Ambient = *(color*)&fxd->paramsf[1];
					l.Diffuse = *(color*)&fxd->paramsf[5];
					l.Specular = *(color*)&fxd->paramsf[9];
					light.add(l);
				}
				_fx ++;
			}
		}*/


	mod.num_trias = Trias.num - mod.offset;
	Models.add(mod);
}

void LightmapData::AddTextureLevels(bool modify)
{
	foreach(Model &m, Models){
		if (modify){
			foreach(ModelMaterial &mat, m.orig->Material)
				mat.NumTextures ++;
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

void LightmapData::CreateVertices()
{
	Vertices.clear();
	foreach(Model &m, Models){
		int w = m.tex_width;
		int h = m.tex_height;

		for (int i=m.offset;i<m.offset + m.num_trias;i++){
			LightmapData::Triangle &t = Trias[i];
			ModelPolygon &p = m.orig->Surface[t.surf].Polygon[t.poly];
			vector v[3], sv[3];
			vector n = p.TempNormal;
			for (int k=0;k<3;k++){
				int si = p.Side[t.side].Triangulation[k];
				v[k] = m.orig->Vertex[p.Side[si].Vertex].pos;
				sv[k] = p.Side[si].SkinVertex[1];
				sv[k].x *= w;
				sv[k].y *= h;
			}

			// rasterize triangle
			rect r = get_tria_skin_boundary(sv);
			int v_offset = Vertices.num;
			for (int x=r.x1-1;x<r.x2+1;x++)
				for (int y=r.y1-1;y<r.y2+1;y++){
					vector c = vector(x, y, 0);
					float f, g;
					GetBaryCentric(c, sv[0], sv[1], sv[2], f, g);
					if ((f >= 0) && (g >= 0) && (f + g <= 1)){
						Vertex vv;
						vv.pos = v[0] + f * (v[1] - v[0]) + g * (v[2] - v[0]);
						vv.n = n;
						vv.x = x;
						vv.y = y;
						vv.tria_id = i;
						vv.mod_id = t.mod_id;
						vv.am = m.orig->Material[m.orig->Surface[t.surf].Polygon[t.poly].Material].Ambient;
						vv.dif = m.orig->Material[m.orig->Surface[t.surf].Polygon[t.poly].Material].Diffuse;
						vv.em = t.em;
						Vertices.add(vv);
					}
				}

			// guess vertex areas
			for (int j=v_offset;j<Vertices.num;j++)
				Vertices[j].area = t.area / (Vertices.num - v_offset);
		}
	}
	msg_write("Vertices: " + i2s(Vertices.num));
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

	Ray r = Ray(a.pos, b.pos);

	for (int ti=0;ti<Trias.num;ti++){
		if ((ti == a.tria_id) || (ti == b.tria_id))
			continue;
		LightmapData::Triangle &t = Trias[ti];
		vector cp;
/*		if (VecLineDistance(tria[t].m, p, p2) > tria[t].r)
		//if (_vec_line_distance_(tria[t].m, p1, p2) > tria[t].r)
			continue;*/
		if (!t.intersect(r, cp))
			continue;

		if (_vec_between_(cp, a.pos, b.pos))
			return false;
	}
	return true;
}

