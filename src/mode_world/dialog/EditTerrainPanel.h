//
// Created by Michael Ankele on 2025-04-21.
//

#ifndef EDITTERRAINPANEL_H
#define EDITTERRAINPANEL_H


#include <lib/xhui/Panel.h>
#include <lib/pattern/Observable.h>

class ModeEditTerrain;

class EditTerrainPanel : public obs::Node<xhui::Panel> {
public:
	explicit EditTerrainPanel(ModeEditTerrain* mode);

	ModeEditTerrain* mode_terrain;
};



#endif //EDITTERRAINPANEL_H
