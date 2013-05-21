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
				t.em = m->Material[p.Material].Emission;
				t.area = ((t.v[1] - t.v[0]) ^ (t.v[2] - t.v[0])).length() / 2;
				area += t.area;
				Trias.add(t);
			}
		}
	}



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

void LightmapData::AddTextureLevels()
{
	foreach(Model &m, Models){
		foreach(ModelMaterial &mat, m.orig->Material){
			mat.NumTextures ++;
		}
		m.orig->Automap(-1, 1); // TODO...
	}
}

