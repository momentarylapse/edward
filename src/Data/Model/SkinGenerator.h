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
	void init_affine(const vector &pos00, const vector &pos01, const vector &pos11);
	void init_projective(const matrix &_m);
	void init_projective(MultiView *mv, int win);

	vector get(const vector &v);

private:
	matrix m;
};

#endif /* SKINGENERATOR_H_ */
