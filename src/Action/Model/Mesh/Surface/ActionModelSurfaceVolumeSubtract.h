/*
 * ActionModelSurfaceVolumeSubtract.h
 *
 *  Created on: 03.06.2012
 *      Author: michi
 */

#ifndef ACTIONMODELSURFACEVOLUMESUBTRACT_H_
#define ACTIONMODELSURFACEVOLUMESUBTRACT_H_

#include "../../../ActionGroup.h"

class ActionModelSurfaceVolumeSubtract : public ActionGroup {
public:
	ActionModelSurfaceVolumeSubtract(int view_stage);
	string name() override { return "ModelSurfaceVolumeSubtract"; }

	void *compose(Data *d) override;
	int view_stage;
};

#endif /* ACTIONMODELSURFACEVOLUMESUBTRACT_H_ */
