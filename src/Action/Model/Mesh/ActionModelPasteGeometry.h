/*
 * ActionModelPasteGeometry.h
 *
 *  Created on: 07.06.2012
 *      Author: michi
 */

#ifndef ACTIONMODELPASTEGEOMETRY_H_
#define ACTIONMODELPASTEGEOMETRY_H_

#include "../../ActionGroup.h"
class DataModel;
class ModeModelGeometry;

class ActionModelPasteGeometry : public ActionGroup
{
public:
	ActionModelPasteGeometry(DataModel *m, ModeModelGeometry &geo);
	virtual ~ActionModelPasteGeometry();
	string name(){	return "ModelPasteGeometry";	}
};

#endif /* ACTIONMODELPASTEGEOMETRY_H_ */
