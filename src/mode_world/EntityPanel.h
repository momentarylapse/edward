//
// Created by michi on 02.02.25.
//

#ifndef ENTITYPANEL_H
#define ENTITYPANEL_H

#include <lib/xhui/Panel.h>
#include <lib/pattern/Observable.h>

struct WorldEntity;
class ModeWorld;

class EntityPanel : public obs::Node<xhui::Panel> {
public:
	explicit EntityPanel(ModeWorld* mode);

	void on_edit_light();
	void on_edit_camera();

	ModeWorld* mode_world;
	int cur_index = -1;
};



#endif //ENTITYPANEL_H
