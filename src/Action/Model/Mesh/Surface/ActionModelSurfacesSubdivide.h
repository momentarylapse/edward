/*
 * ActionModelSurfacesSubdivide.h
 *
 *  Created on: 05.01.2013
 *      Author: michi
 */

#ifndef ACTIONMODELSURFACESSUBDIVIDE_H_
#define ACTIONMODELSURFACESSUBDIVIDE_H_

#include "../../../ActionGroup.h"
#include "../../../../lib/math/math.h"
class DataModel;
class ModelSurface;
class ModelSelection;

class ActionModelSurfacesSubdivide : public ActionGroup {
public:
	ActionModelSurfacesSubdivide(const ModelSelection &sel);
	string name() override { return "ModelSurfacesSubdivide"; }

	void *compose(Data *d) override;
private:
	const ModelSelection &sel;
};

#endif /* ACTIONMODELSURFACESSUBDIVIDE_H_ */
