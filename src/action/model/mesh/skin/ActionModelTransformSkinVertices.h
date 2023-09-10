/*
 * ActionModelTransformSkinVertices.h
 *
 *  Created on: 15.03.2012
 *      Author: michi
 */

#ifndef ACTIONMODELTRANSFORMSKINVERTICES_H_
#define ACTIONMODELTRANSFORMSKINVERTICES_H_

#include "../../../ActionMultiView.h"
class vec3;
class DataModel;

class ActionModelTransformSkinVertices: public ActionMultiView {
public:
	ActionModelTransformSkinVertices(DataModel *d, const Array<int> &tria, const Array<int> &index, int texture_level);
	string name() override { return "ModelTransformSkinVertices"; }

	void *execute(Data *d) override;
	void undo(Data *d) override;

private:
	int texture_level;
	Array<int> tria;
};

#endif /* ACTIONMODELTRANSFORMSKINVERTICES_H_ */
