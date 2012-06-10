/*
 * ActionModelDeleteSurface.h
 *
 *  Created on: 12.03.2012
 *      Author: michi
 */

#ifndef ACTIONMODELDELETESURFACE_H_
#define ACTIONMODELDELETESURFACE_H_

#include "../../../Action.h"
#include "../../../ActionGroup.h"
#include "../../../../lib/types/types.h"

class ActionModelDeleteSurface: public ActionGroup
{
public:
	ActionModelDeleteSurface();
	virtual ~ActionModelDeleteSurface();
	string name(){	return "ModelDeleteSurface";	}
};

#endif /* ACTIONMODELDELETESURFACE_H_ */
