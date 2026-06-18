//
// Created by michi on 9/30/25.
//

#pragma once

#include <lib/xhui/Panel.h>
#include <lib/pattern/Observable.h>

class DataWorld;
class CameraPreviewRenderer;

class CameraPanel : public obs::Node<xhui::Panel> {
public:
	explicit CameraPanel(DataWorld* _data, int _index);
	~CameraPanel() override;
	DataWorld* data;
	int index;
	bool user_editing = false;
	owned<CameraPreviewRenderer> preview;

	void update_ui();
	void on_edit();
};
