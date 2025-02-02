//
// Created by michi on 02.02.25.
//

#include "EntityPanel.h"

#include <view/MultiView.h>

#include "ModeWorld.h"

EntityPanel::EntityPanel(ModeWorld* _mode) : obs::Node<xhui::Panel>("entity-panel") {
	mode = _mode;
	from_source(R"foodelim(
Dialog entity-panel ''
	Grid ? ''
		Grid ? ''
			SpinButton pos-x '' range=::0.001
			SpinButton pos-y '' range=::0.001
			SpinButton pos-z '' range=::0.001
		---|
		Grid ? ''
			SpinButton ang-x '' range=::0.001
			SpinButton ang-y '' range=::0.001
			SpinButton ang-z '' range=::0.001
		---|
		Button a 'test'
)foodelim");
	expand_x = false;
	min_width_user = 400;

	mode->multi_view->out_selection_changed >> create_sink([this] {
		auto sel = mode->data->get_selection();
		bool ok = (sel[MultiViewType::WORLD_ENTITY].num == 1);
		enable("pos-x", ok);
		enable("pos-y", ok);
		enable("pos-z", ok);
		enable("ang-x", ok);
		enable("ang-y", ok);
		enable("ang-z", ok);
		if (sel[MultiViewType::WORLD_ENTITY].num == 1) {
			auto& e = mode->data->entities[sel[MultiViewType::WORLD_ENTITY][0]];
			set_float("pos-x", e.pos.x);
			set_float("pos-y", e.pos.y);
			set_float("pos-z", e.pos.z);
			auto ang = e.ang.get_angles();
			set_float("ang-x", e.ang.x * 180 / pi);
			set_float("ang-y", e.ang.y * 180 / pi);
			set_float("ang-z", e.ang.z * 180 / pi);
		}
	});
	mode->data->out_changed >> create_sink([this] {
		auto sel = mode->data->get_selection();
		bool ok = (sel[MultiViewType::WORLD_ENTITY].num == 1);
		enable("pos-x", ok);
		enable("pos-y", ok);
		enable("pos-z", ok);
		enable("ang-x", ok);
		enable("ang-y", ok);
		enable("ang-z", ok);
		if (sel[MultiViewType::WORLD_ENTITY].num == 1) {
			auto& e = mode->data->entities[sel[MultiViewType::WORLD_ENTITY][0]];
			set_float("pos-x", e.pos.x);
			set_float("pos-y", e.pos.y);
			set_float("pos-z", e.pos.z);
			auto ang = e.ang.get_angles();
			set_float("ang-x", e.ang.x * 180 / pi);
			set_float("ang-y", e.ang.y * 180 / pi);
			set_float("ang-z", e.ang.z * 180 / pi);
		}
	});
}


