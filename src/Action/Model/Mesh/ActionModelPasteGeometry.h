/*
 * ActionModelPasteGeometry.h
 *
 *  Created on: 07.06.2012
 *      Author: michi
 */

#ifndef ACTIONMODELPASTEGEOMETRY_H_
#define ACTIONMODELPASTEGEOMETRY_H_

#include "../../ActionGroup.h"
#include "../../../Data/Model/DataModel.h"

class ActionModelPasteGeometry : public ActionGroup
{
public:
	ActionModelPasteGeometry(DataModel *m, ModeModelGeometry &geo);
	virtual ~ActionModelPasteGeometry();
};

#endif /* ACTIONMODELPASTEGEOMETRY_H_ */
