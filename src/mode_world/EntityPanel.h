//
// Created by michi on 02.02.25.
//

#ifndef ENTITYPANEL_H
#define ENTITYPANEL_H

#include <lib/xhui/Panel.h>
#include <lib/pattern/Observable.h>

struct WorldEntity;
class ModeWorld;

class AddEntityPanel;

class EntityPanel : public obs::Node<xhui::Panel> {
public:
	explicit EntityPanel(ModeWorld* mode);

	ModeWorld* mode_world;
	int cur_index = -1;

	shared<xhui::Panel> add_entity_panel;
	Array<xhui::Panel*> component_panels;
};



#endif //ENTITYPANEL_H
