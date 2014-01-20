/*
 * GeometrySphere.h
 *
 *  Created on: 11.01.2013
 *      Author: michi
 */

#ifndef GEOMETRYSPHERE_H_
#define GEOMETRYSPHERE_H_

#include "Geometry.h"
class vector;

class GeometrySphere : public Geometry
{
public:
	GeometrySphere(const vector &pos, float radius, int num);
	void _cdecl __init__(const vector &pos, float radius, int num);
};

#endif /* GEOMETRYSPHERE_H_ */
