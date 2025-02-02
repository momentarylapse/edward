//
// Created by michi on 02.02.25.
//

#ifndef ENTITYPANEL_H
#define ENTITYPANEL_H

#include <lib/xhui/Panel.h>
#include <lib/pattern/Observable.h>

class ModeWorld;

class EntityPanel : public obs::Node<xhui::Panel> {
public:
	explicit EntityPanel(ModeWorld* mode);

	ModeWorld* mode;
};



#endif //ENTITYPANEL_H
