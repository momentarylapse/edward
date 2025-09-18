//
// Created by Michael Ankele on 2025-02-07.
//

#ifndef MODEMODEL_H
#define MODEMODEL_H


#include "../view/Mode.h"
#include "data/DataModel.h"

class ModeMesh;
class ModeSkeleton;

class ModeModel : public Mode {
public:
	explicit ModeModel(DocumentSession* doc);
	~ModeModel() override;

	void on_enter_rec() override;
	void on_leave_rec() override;
	void on_command(const string& id) override;

	void update_menu();

	owned<DataModel> data;
	owned<ModeMesh> mode_mesh;
	owned<ModeSkeleton> mode_skeleton;
	Array<int> event_ids_rec;
};



#endif //MODEMODEL_H
