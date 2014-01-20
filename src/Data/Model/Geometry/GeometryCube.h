/*
 * GeometryCube.h
 *
 *  Created on: 25.05.2013
 *      Author: michi
 */

#ifndef GEOMETRYCUBE_H_
#define GEOMETRYCUBE_H_

#include "Geometry.h"

class GeometryCube : public Geometry
{
public:
	GeometryCube(const vector &pos, const vector &dv1, const vector &dv2, const vector &dv3, int num_1, int num_2, int num_3);
	void _cdecl __init__(const vector &pos, const vector &dv1, const vector &dv2, const vector &dv3, int num_1, int num_2, int num_3);
};

#endif /* GEOMETRYCUBE_H_ */
