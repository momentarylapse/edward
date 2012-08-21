/*
 * SkinGenerator.cpp
 *
 *  Created on: 19.08.2012
 *      Author: michi
 */

#include "SkinGenerator.h"
#include "../../MultiView.h"

SkinGenerator::SkinGenerator()
{
	m = m_id;
}

SkinGenerator::~SkinGenerator()
{
}

void SkinGenerator::init_affine(const vector& pos00, const vector& pos01,
		const vector& pos11)
{
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

vector SkinGenerator::get(const vector& v)
{
	vector p = m * v;
	p.z = 0;
	return p / (v.x*m._30 + v.y*m._31 + v.z*m._32 + m._33);
}


