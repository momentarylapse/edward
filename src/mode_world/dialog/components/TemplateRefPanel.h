//
// Created by michi on 6/22/26.
//

#pragma once

#include <lib/xhui/Panel.h>
#include <lib/pattern/Observable.h>

class DataWorld;
class MaterialSelector;

class TemplateRefPanel : public obs::Node<xhui::Panel> {
public:
	explicit TemplateRefPanel(DataWorld* _data, int _index);
	void update_ui();
	DataWorld* data;
	int index;
};
