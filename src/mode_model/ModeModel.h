//
// Created by Michael Ankele on 2025-02-07.
//

#ifndef MODEMODEL_H
#define MODEMODEL_H


#include "../view/Mode.h"
#include "data/DataModel.h"

class ModeMesh;

class ModeModel : public Mode {
public:
	explicit ModeModel(Session* session);
	~ModeModel() override;

	void on_enter() override;
	void on_leave() override;

	owned<DataModel> data;
	owned<ModeMesh> mode_mesh;
};



#endif //MODEMODEL_H
