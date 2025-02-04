//
// Created by Michael Ankele on 2025-02-04.
//

#include "AddEntityPanel.h"

#include <lib/os/msg.h>
#include <view/MultiView.h>

#include "ModeWorld.h"

AddEntityPanel::AddEntityPanel(ModeWorld* _mode) : obs::Node<xhui::Panel>("add-entity-panel") {
	mode_world = _mode;
	from_source(R"foodelim(
Dialog add-entity-panel ''
	Grid ? '' expandx
		Label ? 'ghj'
)foodelim");
	size_mode_y = SizeMode::Shrink;
	min_width_user = 320;
}
