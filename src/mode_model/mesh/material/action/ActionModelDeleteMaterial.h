/*
 * ActionModelDeleteMaterial.h
 *
 *  Created on: Dec 16, 2020
 *      Author: michi
 */

#ifndef SRC_ACTION_MODEL_DATA_ACTIONMODELDELETEMATERIAL_H_
#define SRC_ACTION_MODEL_DATA_ACTIONMODELDELETEMATERIAL_H_

#include <lib/history/Action.h>

#include "lib/base/pointer.h"

namespace yrenderer {
	class Material;
}

class ActionModelDeleteMaterial : public history::Action {
public:
	explicit ActionModelDeleteMaterial(int index);
	string name() const override { return "ModelDeleteMaterial"; }

	void *execute(history::Data* d) override;
	void undo(history::Data* d) override;
private:
	int index;
	owned<yrenderer::Material> material;
};

#endif /* SRC_ACTION_MODEL_DATA_ACTIONMODELDELETEMATERIAL_H_ */
