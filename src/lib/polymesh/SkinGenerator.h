/*
 * SkinGenerator.h
 *
 *  Created on: 19.08.2012
 *      Author: michi
 */

#ifndef SKINGENERATOR_H_
#define SKINGENERATOR_H_

#include <lib/math/vec3.h>
#include <lib/math/mat4.h>

struct vec3;
//class MultiViewWindow;

namespace polymesh {
struct Polygon;
struct Vertex;

class SkinGenerator {
public:
	SkinGenerator();
	virtual ~SkinGenerator();
	void init_affine(const vec3 &dir_u, float f_u, const vec3 &dir_v, float f_v);
	void init_projective(const mat4 &_m);
//	void init_projective(MultiViewWindow *win);
	void init_polygon(const Array<Vertex> &v, const Polygon &p);
	void init_point_cloud_boundary(const Array<Vertex> &p, const Array<int> &v);

	vec3 get(const vec3 &v) const;

private:
	mat4 m;
};

}

#endif /* SKINGENERATOR_H_ */
