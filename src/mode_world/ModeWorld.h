//
// Created by Michael Ankele on 2025-01-20.
//

#pragma once

#include <lib/base/optional.h>
#include "../view/Mode.h"
#include "../view/Hover.h"
#include "data/DataWorld.h"
#include <y/world/LevelData.h>


namespace yrenderer {
	struct Light;
}
class MultiViewWindow;
class EntityPanel;
class ModeScripting;
class ModeWorldProperties;
namespace xhui {
	class Panel;
}

class ModeWorld : public Mode {
public:
	explicit ModeWorld(DocumentSession* session);

	void on_enter_rec() override;
	void on_leave_rec() override;
	void on_enter() override;
	void on_leave() override;
	void update_menu();

	void on_mouse_move(const vec2& m, const vec2& d) override;
	void on_mouse_leave(const vec2&) override;
	void on_left_button_down(const vec2&) override;
	void on_left_button_up(const vec2&) override;
	void on_key_down(int key) override;

	void on_prepare_scene(const yrenderer::RenderParams& params) override;
	void on_draw_background(const yrenderer::RenderParams& params, yrenderer::RenderViewData& rvd) override;
	void on_draw_win(const yrenderer::RenderParams& params, MultiViewWindow* win) override;
	void on_draw_shadow(const yrenderer::RenderParams& params, yrenderer::RenderViewData& rvd) override;
	void draw_objects(const yrenderer::RenderParams& params, MultiViewWindow* win);
	void draw_terrains(const yrenderer::RenderParams& params, MultiViewWindow* win);
	void draw_cameras(MultiViewWindow* win);
	void draw_lights(MultiViewWindow* win);

	void on_draw_post(Painter*) override;
	void optimize_view();

	void on_command(const string& id) override;
	void on_set_menu() override;

	DataWorld* data;
	LevelData temp;
	Array<yrenderer::Light*> lights;

	Array<int> event_ids_rec;
	Array<int> event_ids;
	owned<ModeScripting> mode_scripting;
	owned<ModeWorldProperties> mode_properties;

	base::optional<Hover> get_hover(MultiViewWindow* win, const vec2& m) const;

	Data::Selection get_selection(MultiViewWindow* win, const rect& r) const;
};

