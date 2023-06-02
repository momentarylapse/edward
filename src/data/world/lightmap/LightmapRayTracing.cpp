/*
 * LightmapRayTracing.cpp
 *
 *  Created on: 21.05.2013
 *      Author: michi
 */

#include "LightmapRayTracing.h"
#include "../../../stuff/Progress.h"
#include "../../../Edward.h"

static float d_max = 20.0f;
static float r_min = d_max * 0.5f;
static float reflection = 0.9f;

LightmapRayTracing::LightmapRayTracing(LightmapData *_data) :
	Lightmap(_data)
{
}

LightmapRayTracing::~LightmapRayTracing()
{
}

void LightmapRayTracing::Compute()
{
	Array<int> vem;
	foreachi(LightmapData::Vertex &v, data->Vertices, vi){
		v.rad = v.em + v.am * data->Ambient;
		if (v.em.r + v.em.g + v.em.b > 0)
			vem.add(vi);
	}

	// emitting vertices
	foreachi(int vi, vem, ii){
		LightmapData::Vertex &a = data->Vertices[vi];
		for (LightmapData::Vertex &b: data->Vertices){
			if (data->IsVisible(a, b)){


				vec3 d = b.pos - a.pos;
				float r = d.length();
				if (r < r_min)
					r = r_min;
				//msg_write(f2s(r, 3));
				d /= r;
				float f;//=1/(pi*r*r)*vector::dot(d,n1)*vector::dot(d,n2)*d_max*d_max/2;
				f = reflection / pi / (r*r) * vec3::dot(d, a.n) * vec3::dot(d, b.n);
				//f = reflection / pi * pow(r, -1.9) *0.5f * vector::dot(d, v->n) * vector::dot(d, w->n);
				if (f < 0)
					f = -f;

				b.rad += a.em * f * a.area * data->emissive_brightness;
			}
		}
		ed->progress->set(format(_("%d of %d"), ii, vem.num), (float)ii / (float)vem.num);
		if (ed->progress->is_cancelled())
			throw AbortException();
	}

	// lights
	for (LightmapData::Light &l: data->Lights){
		if (l.Directional && (!data->allow_sun))
			continue;
		vec3 p = l.Pos;
		foreachi(LightmapData::Vertex &v, data->Vertices, vi){
			if ((vi & 255) == 0){
				ed->progress->set(format(_("%d of %d"), vi, data->Vertices.num), (float)vi / (float)data->Vertices.num);
				if (ed->progress->is_cancelled())
					throw AbortException();
			}
			if (l.Directional){
				v.rad += v.am * l.Ambient;
				p = v.pos + l.Dir * data->large_distance;
			}
			if (!data->IsVisible(v.pos, p, v.tria_id, -1))
				continue;
			if (l.Directional){
				v.rad += (v.dif * l.Diffuse) * vec3::dot(v.n, l.Dir);
			}else{
			}
			v.rad = White;
		}
	}
}

