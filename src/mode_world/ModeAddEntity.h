//
// Created by Michael Ankele on 2025-02-04.
//

#ifndef MODEADDENTITY_H
#define MODEADDENTITY_H


#include "../view/Mode.h"
#include "data/DataWorld.h"

class MultiViewWindow;
class ModeWorld;

class ModeAddEntity : public Mode {
public:
	explicit ModeAddEntity(ModeWorld* parent);

	void on_enter() override;
	void on_prepare_scene(const RenderParams& params) override;
	void on_draw_win(const RenderParams& params, MultiViewWindow* win) override;

	ModeWorld* mode_world;
	Mode* get_parent() override;
};



#endif //MODEADDENTITY_H
