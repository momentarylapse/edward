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
	ActionModelTransformSkinVertices(DataModel *d, int texture_level);
	string name() override { return "ModelTransformSkinVertices"; }
	const string &message() override;

	void *execute(Data *d) override;
	void undo(Data *d) override;

private:
	int texture_level;
	Array<int> tria;
};

#endif /* ACTIONMODELTRANSFORMSKINVERTICES_H_ */
