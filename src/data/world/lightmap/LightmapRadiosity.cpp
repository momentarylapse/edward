/*
 * LightmapRadiosity.cpp
 *
 *  Created on: 22.05.2013
 *      Author: michi
 */

#include "LightmapRadiosity.h"
#include "../DataWorld.h"
#include "../../../stuff/Progress.h"
#include "../../../Session.h"


static float d_max = 20.0f;
static float r_min = d_max * 0.5f;
static float reflection = 0.9f;

LightmapRadiosity::LightmapRadiosity(LightmapData *_data) :
	Lightmap(_data)
{
}

LightmapRadiosity::~LightmapRadiosity()
{
}

void LightmapRadiosity::Compute()
{
	foreachi(LightmapData::Vertex &a, data->Vertices, i_a){
		a.rad = a.em * data->emissive_brightness;
		foreachi(LightmapData::Vertex &b, data->Vertices, i_b){
			if (i_b <= i_a)
				continue;
			if (data->IsVisible(a, b)){
				a.visible.add(i_b);


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

				//msg_write(f2s(f, 6));
				a.coeff.add(f);
			}
		}
		data->source_world->session->progress->set(format(_("%d of %d"), i_a, data->Vertices.num), sqrt((float)i_a / (float)data->Vertices.num));
		if (data->source_world->session->progress->is_cancelled())
			throw AbortException();
	}

	for (int i=0;i<10;i++)
		Iterate();
}

void LightmapRadiosity::Iterate()
{

	// reset
	for (LightmapData::Vertex &v: data->Vertices)
		v._rad2 = v.em * data->emissive_brightness;

	// iterate
	for (LightmapData::Vertex &a: data->Vertices){
		foreachi(int i, a.visible, ii){
			LightmapData::Vertex &b = data->Vertices[i];
			/*msg_write(f2s(a.coeff[ii]  * b.area, 6));
			msg_write((b.rad * a.dif).str());*/
			a._rad2 += b.rad * a.dif * a.coeff[ii] * b.area;
			//if (raster_visibility)
			b._rad2 += a.rad * b.dif * a.coeff[ii] * a.area;
			//v_rad[i]+=v_em[j]*0.005f;
			//v_rad[j]+=v_em[i]*0.005f;
		}
	}

	/*if (data->allow_sun)
		AddLight();*/

	// set and check...
	for (LightmapData::Vertex &v: data->Vertices){
		v.rad = v._rad2;
		/*if (v.rad.r > 5)
			v.rad.r = 5;
		if (v.rad.g > 5)
			v.rad.g = 5;
		if (v.rad.b > 5)
			v.rad.b = 5;*/
	}
}

