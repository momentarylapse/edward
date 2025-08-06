//
// Created by Michael Ankele on 2025-06-29.
//

#ifndef MODESKELETON_H
#define MODESKELETON_H


#include <lib/base/optional.h>
#include "../../view/Mode.h"
#include "../../view/Hover.h"
#include "../data/DataModel.h"

struct Edge;
class MultiViewWindow;
namespace xhui {
	class Panel;
}
class ModeModel;

class ModeSkeleton : public SubMode {
public:
	explicit ModeSkeleton(ModeModel* parent);
	~ModeSkeleton() override;

	void on_enter_rec() override;
	void on_leave_rec() override;
	void on_enter() override;
	void on_leave() override;
	void on_prepare_scene(const yrenderer::RenderParams& params) override;
	void on_draw_background(const yrenderer::RenderParams& params, yrenderer::RenderViewData& rvd) override;
	void on_draw_win(const yrenderer::RenderParams& params, MultiViewWindow* win) override;
	void on_draw_post(Painter*) override;
	void on_command(const string& id) override;
	void on_key_down(int key) override;
	void on_mouse_move(const vec2& m, const vec2& d) override;

	void optimize_view();

	DataModel* data;
	ModeModel* parent;

	Array<int> event_ids_rec;
	Array<int> event_ids;

	void update_menu();
	base::optional<Hover> get_hover(MultiViewWindow* win, const vec2& m) const;
	Data::Selection select_in_rect(MultiViewWindow* win, const rect& r);
	base::optional<Box> get_selection_box(const Data::Selection& sel) const;
};

#endif //MODESKELETON_H
