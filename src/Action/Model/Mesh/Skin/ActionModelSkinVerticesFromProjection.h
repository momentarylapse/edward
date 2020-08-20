/*
 * ActionModelSkinVerticesFromProjection.h
 *
 *  Created on: 10.06.2012
 *      Author: michi
 */

#ifndef ACTIONMODELSKINVERTICESFROMPROJECTION_H_
#define ACTIONMODELSKINVERTICESFROMPROJECTION_H_

#include "../../../Action.h"
#include "../../../../x/Material.h"
#include "../../../../Data/Model/SkinGenerator.h"
class DataModel;

namespace MultiView{
	class MultiView;
};

class ActionModelSkinVerticesFromProjection : public Action {
public:
	ActionModelSkinVerticesFromProjection(DataModel *m, MultiView::MultiView *mv);
	string name() override { return "ModelSkinVerticesFromProjection"; }
	const string &message() override;

	void *execute(Data *d) override;
	void undo(Data *d) override;

private:
	Array<int> tria;
	Array<int> vert_on_tria;
	Array<vector> old_pos[MATERIAL_MAX_TEXTURES];
	SkinGenerator sg;
};

#endif /* ACTIONMODELSKINVERTICESFROMPROJECTION_H_ */
