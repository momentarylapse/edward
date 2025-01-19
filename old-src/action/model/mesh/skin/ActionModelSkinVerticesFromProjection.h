/*
 * ActionModelSkinVerticesFromProjection.h
 *
 *  Created on: 10.06.2012
 *      Author: michi
 */

#ifndef ACTIONMODELSKINVERTICESFROMPROJECTION_H_
#define ACTIONMODELSKINVERTICESFROMPROJECTION_H_

#include "../../../Action.h"
#include <y/world/Material.h>
#include "../../../../data/model/SkinGenerator.h"
class DataModel;

namespace MultiView{
	class MultiView;
};

class ActionModelSkinVerticesFromProjection : public Action {
public:
	ActionModelSkinVerticesFromProjection(DataModel *m, MultiView::MultiView *mv);
	string name() override { return "ModelSkinVerticesFromProjection"; }

	void *execute(Data *d) override;
	void undo(Data *d) override;

private:
	Array<int> tria;
	Array<int> vert_on_tria;
	Array<vec3> old_pos[MATERIAL_MAX_TEXTURES];
	SkinGenerator sg;
};

#endif /* ACTIONMODELSKINVERTICESFROMPROJECTION_H_ */
