/*
 * SkinGenerator.cpp
 *
 *  Created on: 19.08.2012
 *      Author: michi
 */

#include "../../lib/types/types.h"
#include "SkinGenerator.h"
#include "../../MultiView.h"
#include "../../Data/Model/DataModel.h"

SkinGenerator::SkinGenerator()
{
	m = m_id;
}

SkinGenerator::~SkinGenerator()
{
}

void SkinGenerator::init_affine(const vector &dir_u, float f_u, const vector &dir_v, float f_v)
{
	m = m_id;
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

void SkinGenerator::init_projective(MultiView *mv, int win)
{
	irect d = mv->view[win].dest;
	matrix s, t1, t2;
	MatrixScale(s, MaxX / (float)(d.x2 - d.x1) / 2, - MaxY / (float)(d.y2 - d.y1) / 2, 1);
	MatrixTranslation(t2, vector(- (float)d.x1 / MaxX * 2, - (float)d.y1 / MaxY * 2, 0));
	MatrixTranslation(t1, vector(1, -1, 0));
	init_projective(t2 * s * t1 * mv->view[win].projection * mv->view[win].mat);
}

void SkinGenerator::init_polygon(DataModel *model, ModelPolygon &p)
{
	vector n = p.TempNormal;
	vector d1 = n.ortho();
	vector d2 = n ^ d1;
	float sx = 0, sy = 0, sxx = 0, syy = 0, sxy = 0, su = 0, sv = 0, sux = 0, suy = 0, svx = 0, svy = 0;
	foreach(ModelPolygonSide &s, p.Side){
		float x = d1 * model->Vertex[s.Vertex].pos;
		float y = d2 * model->Vertex[s.Vertex].pos;
		float u = s.SkinVertex[0].x;
		float v = s.SkinVertex[0].y;
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
	matrix mm = m_id;
	mm._00 = sxx;
	mm._01 = sxy;
	mm._02 = sx;
	mm._10 = sxy;
	mm._11 = syy;
	mm._12 = sy;
	mm._20 = sx;
	mm._21 = sy;
	mm._22 = p.Side.num;
	matrix imm;
	MatrixInverse(imm, mm);
	vector uu = imm * vector(sux, suy, su);
	vector vv = imm * vector(svx, svy, sv);
	m = m_id;
	m._00 = uu.x;
	m._01 = uu.y;
	m._03 = uu.z;
	m._10 = vv.x;
	m._11 = vv.y;
	m._13 = vv.z;
	m._22 = 0;
}

vector SkinGenerator::get(const vector& v) const
{
	vector p = m.project(v);
	p.z = 0;
	return p;
}


