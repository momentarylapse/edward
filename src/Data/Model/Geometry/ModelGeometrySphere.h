/*
 * ModelGeometrySphere.h
 *
 *  Created on: 11.01.2013
 *      Author: michi
 */

#ifndef MODELGEOMETRYSPHERE_H_
#define MODELGEOMETRYSPHERE_H_

#include "ModelGeometry.h"
class vector;

class ModelGeometrySphere : public ModelGeometry
{
public:
	ModelGeometrySphere(const vector &pos, float radius, int num);
};

#endif /* MODELGEOMETRYSPHERE_H_ */
