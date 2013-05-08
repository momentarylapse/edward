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
template<class T>
class Set;

class ActionModelSurfacesSubdivide : public ActionGroup
{
public:
	ActionModelSurfacesSubdivide(const Set<int> &surfaces);
	virtual ~ActionModelSurfacesSubdivide(){};
	string name(){	return "ModelSurfacesSubdivide";	}

	void *compose(Data *d);
private:
	void SubdivideSurface(DataModel *m, ModelSurface *s, int surface);
	const Set<int> &surfaces;
};

#endif /* ACTIONMODELSURFACESSUBDIVIDE_H_ */
