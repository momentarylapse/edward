//
// Created by michi on 9/30/25.
//

#pragma once

#include "ComponentPanel.h"

class DataWorld;
class CameraPreviewRenderer;

class CameraPanel : public ComponentContentsPanel {
public:
	explicit CameraPanel(DataWorld* _data, int _index);
	~CameraPanel() override;
	owned<CameraPreviewRenderer> preview;

	void update_ui() override;
	void on_edit();
};
