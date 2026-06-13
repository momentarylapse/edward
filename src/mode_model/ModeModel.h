//
// Created by Michael Ankele on 2025-02-07.
//

#pragma once


#include <view/Mode.h>
#include "data/DataModel.h"

class ModeMesh;
class ModeSkeleton;
class MultiViewPanel;

class ModeModel : public Mode {
public:
	explicit ModeModel(DocumentSession* doc);
	~ModeModel() override;

	void on_enter() override;
	void on_enter_rec() override;
	void on_connect_events_rec() override;
	void on_leave_rec() override;
	void on_command(const string& id) override;

	void on_update_menu() override;

	owned<DataModel> data;
	owned<ModeMesh> mode_mesh;
	owned<ModeSkeleton> mode_skeleton;
	shared<MultiViewPanel> multi_view_panel;
	owned<MultiView> multi_view_3d;
	owned<MultiView> multi_view_2d;
};


