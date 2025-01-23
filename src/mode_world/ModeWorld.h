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
	void on_mouse_move(const vec2& m, const vec2& d) override;
	void on_mouse_leave(const vec2&) override;
	void on_left_button_down(const vec2&) override;
	void on_left_button_up(const vec2&) override;
	void on_key_down(int key) override;
	void on_draw_post(Painter*) override;
	void optimize_view();

	void on_command(const string& id) override;

	DataWorld* data;
	struct Hover {
		int type, index;
	};

	using Selection = base::set<void*>;

	base::optional<Hover> hover;
	base::optional<Hover> get_hover(const vec2& m) const;

	Selection selection;
	Selection get_selection(const rect& r) const;
	void update_selection_box();
};



#endif //MODEWORLD_H
