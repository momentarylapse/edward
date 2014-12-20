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

class ModeWorldCameraCreatePoint: public ModeCreation<DataCamera>
{
public:
	ModeWorldCameraCreatePoint(ModeBase *_parent);

	virtual void onLeftButtonDown();

	string filename;
};

#endif /* MODEWORLDCAMERACREATEPOINT_H_ */
