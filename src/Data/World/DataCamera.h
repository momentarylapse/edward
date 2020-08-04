/*
 * DataCamera.h
 *
 *  Created on: 30.09.2012
 *      Author: michi
 */

#ifndef DATACAMERA_H_
#define DATACAMERA_H_

#include "../Data.h"
#include "../../MultiView/SingleData.h"
#include "../../lib/math/math.h"


#define CPKSetCamPos	0
#define CPKSetCamPosRel	1
#define CPKSetCamAng	2
#define CPKSetCamPosAng	4
#define CPKCamFlight	10
#define CPKCamFlightRel	11

class WorldCamPoint: public MultiView::SingleData {
public:
	int Type;
	vector Vel, Ang;
	float Duration;
};

class WorldCamPointVel: public MultiView::SingleData {
};

class DataCamera : public Data {
public:
	DataCamera();
	virtual ~DataCamera();

	void reset();
	bool load(const Path &_filename, bool deep = true);
	bool save(const Path &_filename);

	void UpdateVel();

	float GetDuration() const;
	Interpolator<vector> BuildPosInterpolator() const;
	Interpolator<vector> BuildAngInterpolator() const;

	Array<WorldCamPoint> Point;
	Array<WorldCamPointVel> Vel;
};

#endif /* DATACAMERA_H_ */
