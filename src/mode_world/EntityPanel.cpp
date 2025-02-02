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
		//if (sel[MD])
	});
}


