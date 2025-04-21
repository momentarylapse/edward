//
// Created by Michael Ankele on 2025-02-04.
//

#ifndef ADDENTITYPANEL_H
#define ADDENTITYPANEL_H


#include <lib/xhui/Panel.h>
#include <lib/pattern/Observable.h>

struct WorldEntity;
class ModeWorld;

class AddEntityPanel : public obs::Node<xhui::Panel> {
public:
	explicit AddEntityPanel(ModeWorld* mode);

	ModeWorld* mode_world;
};




#endif //ADDENTITYPANEL_H
