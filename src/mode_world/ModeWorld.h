//
// Created by Michael Ankele on 2025-01-20.
//

#ifndef MODEWORLD_H
#define MODEWORLD_H

#include "../view/Mode.h"
#include "data/DataWorld.h"

class ModeWorld : public Mode {
public:
	explicit ModeWorld(Session* session);

	Renderer* create_renderer(SceneView* scene_view) override;

	DataWorld* data;
};



#endif //MODEWORLD_H
