/*
 * DataCamera.h
 *
 *  Created on: 30.09.2012
 *      Author: michi
 */

#ifndef DATACAMERA_H_
#define DATACAMERA_H_

#include "../Data.h"
#include "../../MultiView.h"
#include "../../lib/x/x.h"


#define CPKSetCamPos	0
#define CPKSetCamPosRel	1
#define CPKSetCamAng	2
#define CPKSetCamPosAng	4
#define CPKCamFlight	10
#define CPKCamFlightRel	11

class WorldCamPoint: public MultiViewSingleData
{
public:
	int Type;
	vector Vel, Ang;
	float Duration;
};

class WorldCamPointVel: public MultiViewSingleData
{
};

class DataCamera : public Data
{
public:
	DataCamera();
	virtual ~DataCamera();

	void Reset();
	bool Load(const string &_filename, bool deep = true);
	bool Save(const string &_filename);

	void UpdateVel();

	Array<WorldCamPoint> Point;
	Array<WorldCamPointVel> Vel;
};

#endif /* DATACAMERA_H_ */
