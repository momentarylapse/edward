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
class ModeWorldCamera;

class ModeWorldCameraCreatePoint: public ModeCreation<ModeWorldCamera, DataCamera> {
public:
	ModeWorldCameraCreatePoint(ModeWorldCamera *_parent);

	void on_left_button_down() override;

	string filename;
};

#endif /* MODEWORLDCAMERACREATEPOINT_H_ */
