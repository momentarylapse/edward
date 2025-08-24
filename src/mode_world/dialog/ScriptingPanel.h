//
// Created by michi on 8/24/25.
//

#pragma once

#include <lib/xhui/Panel.h>
#include <lib/pattern/Observable.h>

class ModeWorld;

class ScriptingPanel : public obs::Node<xhui::Panel> {
public:
	explicit ScriptingPanel(ModeWorld* mode);

	void fill_list();

	ModeWorld* mode_world;
};
