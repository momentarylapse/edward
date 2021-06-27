/*
 * SkinGenerator.cpp
 *
 *  Created on: 19.08.2012
 *      Author: michi
 */

#include "../../lib/math/math.h"
#include "SkinGenerator.h"
#include "../../MultiView/MultiView.h"
#include "../../MultiView/Window.h"
#include "../../lib/nix/nix.h"
#include "../../Data/Model/ModelMesh.h"
#include "../../Data/Model/ModelPolygon.h"

SkinGenerator::SkinGenerator()
{
	m = matrix::ID;
}

SkinGenerator::~SkinGenerator()
{
}

void SkinGenerator::init_affine(const vector &dir_u, float f_u, const vector &dir_v, float f_v)
{
	m = matrix::ID;
	m._00 = dir_u.x;
	m._01 = dir_u.y;
	m._02 = dir_u.z;
	m._03 = f_u;
	m._10 = dir_v.x;
	m._11 = dir_v.y;
	m._12 = dir_v.z;
	m._13 = f_v;
}

void SkinGenerator::init_projective(const matrix &_m)
{
	m = _m;
}

void SkinGenerator::init_projective(MultiView::Window *win)
{
	rect d = win->dest;
	matrix s, t1, t2;
	s = matrix::scale( nix::target_width / (d.x2 - d.x1) / 2, - nix::target_height / (d.y2 - d.y1) / 2, 1);
	t2 = matrix::translation( vector(- d.x1 / nix::target_width * 2, - d.y1 / nix::target_height * 2, 0));
	t1 = matrix::translation( vector(1, -1, 0));
	init_projective(t2 * s * t1 * win->pv_matrix);
}

void SkinGenerator::init_polygon(const Array<ModelVertex> &v, ModelPolygon &p, int level)
{
	vector n = p.temp_normal;
	vector d1 = n.ortho();
	vector d2 = vector::cross(n, d1);
	matrix R = matrix(d1, d2, n);
	float sx = 0, sy = 0, sxx = 0, syy = 0, sxy = 0, su = 0, sv = 0, sux = 0, suy = 0, svx = 0, svy = 0;
	for (ModelPolygonSide &s: p.side){
		float x = vector::dot(d1, v[s.vertex].pos);
		float y = vector::dot(d2, v[s.vertex].pos);
		float u = s.skin_vertex[level].x;
		float v = s.skin_vertex[level].y;
		sx += x;
		sy += y;
		sxx += x*x;
		syy += y*y;
		sxy += x*y;
		su += u;
		sv += v;
		sux += u*x;
		svx += v*x;
		suy += u*y;
		svy += v*y;
	}
	matrix mm = matrix::ID;
	mm._00 = sxx;
	mm._01 = sxy;
	mm._02 = sx;
	mm._10 = sxy;
	mm._11 = syy;
	mm._12 = sy;
	mm._20 = sx;
	mm._21 = sy;
	mm._22 = p.side.num;
	matrix imm;
	imm = mm.inverse();
	vector uu = imm * vector(sux, suy, su);
	vector vv = imm * vector(svx, svy, sv);
	m = matrix::ID;
	m._00 = uu.x;
	m._01 = uu.y;
	m._03 = uu.z;
	m._10 = vv.x;
	m._11 = vv.y;
	m._13 = vv.z;
	m._22 = 0;
	matrix iR = R.inverse();
	m = m * iR;
}


static vector get_cloud_normal(const Array<ModelVertex> &pp, const Array<int> &v)
{
	Array<vector> p;
	for (int i=1;i<v.num;i++){
		p.add(pp[v[i]].pos - pp[v[0]].pos);
		p.back().normalize();
	}
	for (int i=0;i<p.num;i++)
		for (int j=i+1;j<p.num;j++){
			vector d = vector::cross(p[i], p[j]);
			float l = d.length();
			if (l > 0.1f)
				return d / l;
		}
	return v_0;
}

void SkinGenerator::init_point_cloud_boundary(const Array<ModelVertex> &p, const Array<int> &v)
{
	vector n = get_cloud_normal(p, v);
	vector d[2];
	d[0] = n.ortho();
	d[1] = vector::cross(d[0], n);
	float boundary[2][2], l[2];
	for (int k=0;k<2;k++){
		boundary[k][0] = boundary[k][1] = vector::dot(d[k], p[v[0]].pos);
		for (int vi: const_cast<Array<int>&>(v)){
			float f = vector::dot(d[k], p[vi].pos);
			if (f < boundary[k][0])
				boundary[k][0] = f;
			if (f > boundary[k][1])
				boundary[k][1] = f;
		}
		l[k] = (boundary[k][1] - boundary[k][0]);
	}
	init_affine(d[0] / l[0], - boundary[0][0] / l[0], d[1] / l[1], - boundary[1][0] / l[1]);

}

vector SkinGenerator::get(const vector& v) const
{
	vector p = m.project(v);
	p.z = 0;
	return p;
}

SkinGeneratorMulti::SkinGeneratorMulti()
{
	gen = new SkinGenerator[MATERIAL_MAX_TEXTURES];
}

SkinGeneratorMulti::SkinGeneratorMulti(const SkinGenerator &sg)
{
	gen = new SkinGenerator[MATERIAL_MAX_TEXTURES];
	for (int i=0;i<MATERIAL_MAX_TEXTURES;i++)
		gen[i] = sg;
}

SkinGeneratorMulti::~SkinGeneratorMulti()
{
	delete[](gen);
}

void SkinGeneratorMulti::init_polygon(const Array<ModelVertex> &v, ModelPolygon &p)
{
	for (int i=0;i<MATERIAL_MAX_TEXTURES;i++)
		gen[i].init_polygon(v, p, i);
}

vector SkinGeneratorMulti::get(const vector& v, int level) const
{
	return gen[level].get(v);
}
