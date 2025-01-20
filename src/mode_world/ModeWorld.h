//
// Created by Michael Ankele on 2025-01-20.
//

#ifndef MODEWORLD_H
#define MODEWORLD_H

#include <lib/base/optional.h>
#include "../view/Mode.h"
#include "data/DataWorld.h"

class ModeWorld : public Mode {
public:
	explicit ModeWorld(Session* session);

	Renderer* create_renderer(SceneView* scene_view) override;
	void on_mouse_move(const vec2&) override;
	void on_draw_post(Painter*) override;
	void optimize_view();

	DataWorld* data;
	struct Hover {
		int type, index;
	};

	base::optional<Hover> hover;
};



#endif //MODEWORLD_H
