/*
 * SkinGenerator.h
 *
 *  Created on: 19.08.2012
 *      Author: michi
 */

#ifndef SKINGENERATOR_H_
#define SKINGENERATOR_H_

#include "../../lib/types/matrix.h"
class vector;
class MultiView;
class DataModel;
class ModelPolygon;

class SkinGenerator
{
public:
	SkinGenerator();
	virtual ~SkinGenerator();
	void init_affine(const vector &dir_u, float f_u, const vector &dir_v, float f_v);
	void init_projective(const matrix &_m);
	void init_projective(MultiView *mv, int win);
	void init_polygon(DataModel *model, ModelPolygon &p);

	vector get(const vector &v) const;

private:
	matrix m;
};

#endif /* SKINGENERATOR_H_ */
