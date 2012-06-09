/*
 * ActionModelSurfaceAutoWeld.h
 *
 *  Created on: 03.06.2012
 *      Author: michi
 */

#ifndef ACTIONMODELSURFACEAUTOWELD_H_
#define ACTIONMODELSURFACEAUTOWELD_H_

#include "../../../Action.h"
#include "../../../ActionGroup.h"
#include "../../../../lib/types/types.h"
#include "../../../../Data/Model/DataModel.h"

class ActionModelSurfaceAutoWeld : public ActionGroup
{
public:
	ActionModelSurfaceAutoWeld(DataModel *m, int _surface1, int _surface2, float d);
	virtual ~ActionModelSurfaceAutoWeld();
};

#endif /* ACTIONMODELSURFACEAUTOWELD_H_ */
