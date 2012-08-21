/*
 * SkinGenerator.h
 *
 *  Created on: 19.08.2012
 *      Author: michi
 */

#ifndef SKINGENERATOR_H_
#define SKINGENERATOR_H_

#include "../../lib/types/types.h"
class MultiView;

class SkinGenerator
{
public:
	SkinGenerator();
	virtual ~SkinGenerator();
	void init_affine(const vector &dir_u, float f_u, const vector &dir_v, float f_v);
	void init_projective(const matrix &_m);
	void init_projective(MultiView *mv, int win);

	vector get(const vector &v) const;

private:
	matrix m;
};

#endif /* SKINGENERATOR_H_ */
