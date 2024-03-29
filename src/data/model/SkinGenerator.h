/*
 * SkinGenerator.h
 *
 *  Created on: 19.08.2012
 *      Author: michi
 */

#ifndef SKINGENERATOR_H_
#define SKINGENERATOR_H_

#include "../../lib/math/vec3.h"
#include "../../lib/math/mat4.h"

class vec3;
namespace MultiView{
	class Window;
};
class ModelPolygon;
class ModelVertex;

class SkinGenerator {
public:
	SkinGenerator();
	virtual ~SkinGenerator();
	void init_affine(const vec3 &dir_u, float f_u, const vec3 &dir_v, float f_v);
	void init_projective(const mat4 &_m);
	void init_projective(MultiView::Window *win);
	void init_polygon(const Array<ModelVertex> &v, ModelPolygon &p, int level);
	void init_point_cloud_boundary(const Array<ModelVertex> &p, const Array<int> &v);

	vec3 get(const vec3 &v) const;

private:
	mat4 m;
};

class SkinGeneratorMulti {
public:
	SkinGeneratorMulti();
	SkinGeneratorMulti(const SkinGenerator &sg);
	virtual ~SkinGeneratorMulti();
	void init_polygon(const Array<ModelVertex> &v, ModelPolygon &p);

	vec3 get(const vec3 &v, int level) const;

private:
	SkinGenerator *gen;
};

#endif /* SKINGENERATOR_H_ */
