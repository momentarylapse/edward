/*
 * SkinGenerator.cpp
 *
 *  Created on: 19.08.2012
 *      Author: michi
 */

#include <lib/math/math.h>
#include "SkinGenerator.h"
#include <lib/mesh/PolygonMesh.h>
#if 0 //HAS_LIB_GL
#include "../../multiview/MultiView.h"
#include "../../multiview/Window.h"
#endif
//#include "../../lib/nix/nix.h"
//#include <mode_model/data/ModelMesh.h>

SkinGenerator::SkinGenerator() {
	m = mat4::ID;
}

SkinGenerator::~SkinGenerator() = default;

void SkinGenerator::init_affine(const vec3 &dir_u, float f_u, const vec3 &dir_v, float f_v) {
	m = mat4::ID;
	m._00 = dir_u.x;
	m._01 = dir_u.y;
	m._02 = dir_u.z;
	m._03 = f_u;
	m._10 = dir_v.x;
	m._11 = dir_v.y;
	m._12 = dir_v.z;
	m._13 = f_v;
}

void SkinGenerator::init_projective(const mat4 &_m) {
	m = _m;
}

//void SkinGenerator::init_projective(MultiViewWindow *win) {
#if 0 //HAS_LIB_GL
	rect d = win->dest;
	mat4 s, t1, t2;
	s = mat4::scale( nix::target_width / (d.x2 - d.x1) / 2, - nix::target_height / (d.y2 - d.y1) / 2, 1);
	t2 = mat4::translation( vec3(- d.x1 / nix::target_width * 2, - d.y1 / nix::target_height * 2, 0));
	t1 = mat4::translation( vec3(1, -1, 0));
	init_projective(t2 * s * t1 * win->pv_matrix);
#endif
//}

void SkinGenerator::init_polygon(const Array<MeshVertex> &v, Polygon &p, int level) {
	vec3 n = p.temp_normal;
	vec3 d1 = n.ortho();
	vec3 d2 = vec3::cross(n, d1);
	mat4 R = mat4(d1, d2, n);
	float sx = 0, sy = 0, sxx = 0, syy = 0, sxy = 0, su = 0, sv = 0, sux = 0, suy = 0, svx = 0, svy = 0;
	for (PolygonSide &s: p.side){
		float x = vec3::dot(d1, v[s.vertex].pos);
		float y = vec3::dot(d2, v[s.vertex].pos);
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
	mat4 mm = mat4::ID;
	mm._00 = sxx;
	mm._01 = sxy;
	mm._02 = sx;
	mm._10 = sxy;
	mm._11 = syy;
	mm._12 = sy;
	mm._20 = sx;
	mm._21 = sy;
	mm._22 = p.side.num;
	mat4 imm;
	imm = mm.inverse();
	vec3 uu = imm * vec3(sux, suy, su);
	vec3 vv = imm * vec3(svx, svy, sv);
	m = mat4::ID;
	m._00 = uu.x;
	m._01 = uu.y;
	m._03 = uu.z;
	m._10 = vv.x;
	m._11 = vv.y;
	m._13 = vv.z;
	m._22 = 0;
	mat4 iR = R.inverse();
	m = m * iR;
}


static vec3 get_cloud_normal(const Array<MeshVertex> &pp, const Array<int> &v)
{
	Array<vec3> p;
	for (int i=1;i<v.num;i++){
		p.add(pp[v[i]].pos - pp[v[0]].pos);
		p.back().normalize();
	}
	for (int i=0;i<p.num;i++)
		for (int j=i+1;j<p.num;j++){
			vec3 d = vec3::cross(p[i], p[j]);
			float l = d.length();
			if (l > 0.1f)
				return d / l;
		}
	return v_0;
}

void SkinGenerator::init_point_cloud_boundary(const Array<MeshVertex> &p, const Array<int> &v)
{
	vec3 n = get_cloud_normal(p, v);
	vec3 d[2];
	d[0] = n.ortho();
	d[1] = vec3::cross(d[0], n);
	float boundary[2][2], l[2];
	for (int k=0;k<2;k++){
		boundary[k][0] = boundary[k][1] = vec3::dot(d[k], p[v[0]].pos);
		for (int vi: const_cast<Array<int>&>(v)){
			float f = vec3::dot(d[k], p[vi].pos);
			if (f < boundary[k][0])
				boundary[k][0] = f;
			if (f > boundary[k][1])
				boundary[k][1] = f;
		}
		l[k] = (boundary[k][1] - boundary[k][0]);
	}
	init_affine(d[0] / l[0], - boundary[0][0] / l[0], d[1] / l[1], - boundary[1][0] / l[1]);

}

vec3 SkinGenerator::get(const vec3& v) const
{
	vec3 p = m.project(v);
	p.z = 0;
	return p;
}

#if 0

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

void SkinGeneratorMulti::init_polygon(const Array<MeshVertex> &v, Polygon &p)
{
	for (int i=0;i<MATERIAL_MAX_TEXTURES;i++)
		gen[i].init_polygon(v, p, i);
}

vec3 SkinGeneratorMulti::get(const vec3& v, int level) const
{
	return gen[level].get(v);
}

#endif
