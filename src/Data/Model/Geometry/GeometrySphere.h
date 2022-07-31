/*
 * GeometrySphere.h
 *
 *  Created on: 11.01.2013
 *      Author: michi
 */

#ifndef GEOMETRYSPHERE_H_
#define GEOMETRYSPHERE_H_

#include "Geometry.h"
class vec3;

class GeometrySphere : public Geometry
{
public:
	GeometrySphere(const vec3 &pos, float radius, int num);
	void _cdecl __init__(const vec3 &pos, float radius, int num);
};

#endif /* GEOMETRYSPHERE_H_ */
