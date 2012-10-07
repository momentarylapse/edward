/*
 * ModeWorldCameraCreatePoint.h
 *
 *  Created on: 07.10.2012
 *      Author: michi
 */

#ifndef MODEWORLDCAMERACREATEPOINT_H_
#define MODEWORLDCAMERACREATEPOINT_H_

#include "../../../ModeCreation.h"
class DataCamera;

class ModeWorldCameraCreatePoint: public ModeCreation
{
public:
	ModeWorldCameraCreatePoint(Mode *_parent);
	virtual ~ModeWorldCameraCreatePoint();

	void OnLeftButtonDown();

	DataCamera *data;

	string filename;
};

#endif /* MODEWORLDCAMERACREATEPOINT_H_ */
