/*
 * LightmapPhotonMapImageSpace.cpp
 *
 *  Created on: 19.06.2013
 *      Author: michi
 */

#include "LightmapPhotonMapImageSpace.h"

LightmapPhotonMapImageSpace::LightmapPhotonMapImageSpace(LightmapData *data, int num_photons) :
	LightmapPhotonMap(data, num_photons)
{
}

LightmapPhotonMapImageSpace::~LightmapPhotonMapImageSpace()
{
}

void LightmapPhotonMapImageSpace::CreateTextures()
{
	// create empty
	psi.resize(data->Models.num);
	foreachi(LightmapData::Model &m, data->Models, mid){
		psi[mid].w = m.tex_width;
		psi[mid].h = m.tex_height;
		psi[mid].c.resize(m.tex_width * m.tex_height);
	}

	// fill with photon events
	for (PhotonEvent &p: photon){
		LightmapData::Triangle &t = data->Trias[p.tria];
		vector sv = t.sv[0] + (t.sv[1] - t.sv[0]) * p.f + (t.sv[2] - t.sv[0]) * p.g;
		int x = sv.x + 0.5f;
		int y = sv.y + 0.5f;
		float v_inv_area = t.num_vertices / t.area;
		psi[t.mod_id].c[x + y * psi[t.mod_id].w] += p.c * v_inv_area;
	}

	// set alpha to 0 or 1
	for (PseudoImage &p: psi)
		for (color &c: p.c)
			c.a = 0;
	for (LightmapData::Vertex &v: data->Vertices)
		psi[v.mod_id].c[v.x + v.y * psi[v.mod_id].w].a = 1;
}

void LightmapPhotonMapImageSpace::PrepareTextureRendering()
{
	CreateTextures();
}

color LightmapPhotonMapImageSpace::get_color(LightmapData::Vertex &v, float r)
{
	color c = Black;
	int n = 0;
	PseudoImage &p = psi[v.mod_id];
	int x0  = ::max(v.x - r, 0.0f);
	int x1  = ::min(v.x + r + 1, p.w - 1.0f);
	int y0  = ::max(v.y - r, 0.0f);
	int y1  = ::min(v.y + r + 1, p.h - 1.0f);
	for (int xx=x0; xx<x1; xx++)
		for (int yy=y0; yy<y1; yy++)
			if ((xx - v.x)*(xx - v.x) + (yy - v.y) * (yy - v.y) <= r*r){
				color cc = p.c[xx + yy * p.w];
				if (cc.a > 0){
					c += cc;
					n ++;
				}
			}
	if (((c.r + c.g + c.b) < 30.0) && (r < 5))
		return get_color(v, r + 0.5);
	c *= 1.0f / n;
	c.a = 1;
	return c;
}

color LightmapPhotonMapImageSpace::RenderVertex(LightmapData::Vertex &v)
{
	color r = get_color(v, 0);
	if (v.em.r + v.em.g + v.em.b > 0)
		r = v.em;
	r.clamp();
	r.a = 1;
	if (data->color_exponent != 1){
		r.r = pow(r.r, data->color_exponent);
		r.g = pow(r.g, data->color_exponent);
		r.b = pow(r.b, data->color_exponent);
	}
	return r;
}

