/*
 * SkinGenerator.h
 *
 *  Created on: 19.08.2012
 *      Author: michi
 */

#ifndef SKINGENERATOR_H_
#define SKINGENERATOR_H_

#include "../../lib/math/math.h"
class vector;
class MultiViewWindow;
class DataModel;
class ModelPolygon;
class ModelVertex;

class SkinGenerator
{
public:
	SkinGenerator();
	virtual ~SkinGenerator();
	void init_affine(const vector &dir_u, float f_u, const vector &dir_v, float f_v);
	void init_projective(const matrix &_m);
	void init_projective(MultiViewWindow *win);
	void init_polygon(DataModel *model, ModelPolygon &p, int level);
	void init_point_cloud_boundary(const Array<ModelVertex> &p, const Array<int> &v);

	vector get(const vector &v) const;

private:
	matrix m;
};

class SkinGeneratorMulti
{
public:
	SkinGeneratorMulti();
	SkinGeneratorMulti(const SkinGenerator &sg);
	virtual ~SkinGeneratorMulti();
	void init_polygon(DataModel *model, ModelPolygon &p);

	vector get(const vector &v, int level) const;

private:
	SkinGenerator *gen;
};

#endif /* SKINGENERATOR_H_ */
