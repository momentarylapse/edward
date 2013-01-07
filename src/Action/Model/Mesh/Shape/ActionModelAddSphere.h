/*
 * ActionModelAddSphere.h
 *
 *  Created on: 24.12.2012
 *      Author: michi
 */

#ifndef ACTIONMODELADDSPHERE_H_
#define ACTIONMODELADDSPHERE_H_

#include "../../../ActionGroup.h"
#include "../../../../lib/types/vector.h"

class ActionModelAddSphere: public ActionGroup
{
public:
	ActionModelAddSphere(const vector &pos, float radius, int num);
	virtual ~ActionModelAddSphere(){}
	string name(){	return "ModelAddSphere";	}

	void *compose(Data *d);
private:
	vector pos;
	float radius;
	int num;
};

#endif /* ACTIONMODELADDSPHERE_H_ */