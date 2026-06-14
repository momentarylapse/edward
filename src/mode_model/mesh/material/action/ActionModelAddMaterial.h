/*
 * ActionModelAddMaterial.h
 *
 *  Created on: 06.06.2012
 *      Author: michi
 */

#ifndef ACTIONMODELADDMATERIAL_H_
#define ACTIONMODELADDMATERIAL_H_

#include <lib/history/Action.h>
#include <lib/base/pointer.h>

namespace yrenderer {
	struct Material;
}

class ActionModelAddMaterial : public history::Action {
public:
	explicit ActionModelAddMaterial(xfer<yrenderer::Material> material);
	string name() const override { return "ModelAddMaterial"; }

	void* execute(history::Data* d) override;
	void undo(history::Data* d) override;
private:
	owned<yrenderer::Material> material;
};

#endif /* ACTIONMODELADDMATERIAL_H_ */
