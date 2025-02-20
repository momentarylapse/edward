//
// Created by Michael Ankele on 2025-02-20.
//

#ifndef MODEMESH_H
#define MODEMESH_H



#include <lib/base/optional.h>
#include "../../view/Mode.h"
#include "../../view/Hover.h"
#include "../data/DataModel.h"

class MultiViewWindow;
namespace xhui {
	class Panel;
}
class Material;
class ModeModel;

class ModeMesh : public Mode {
public:
	explicit ModeMesh(ModeModel* parent);
	~ModeMesh() override;

	void on_enter() override;
	void on_leave() override;
	void on_prepare_scene(const RenderParams& params) override;
	void on_draw_win(const RenderParams& params, MultiViewWindow* win) override;
	void on_draw_post(Painter*) override;
	void on_command(const string& id) override;
	void on_key_down(int key) override;
	void on_mouse_move(const vec2& m, const vec2& d) override;

	void on_update_selection();
	void update_vb();
	void update_selection_vb();

	void optimize_view();

	DataModel* data;
	VertexBuffer* vertex_buffer;
	VertexBuffer* vertex_buffer_selection;
	Material* material;
	Material* material_selection;
	Array<int> event_ids;

	enum class PresentationMode {
		Vertices,
		Edges,
		Polygons,
		Surfaces
	} presentation_mode;
	void set_presentation_mode(PresentationMode m);
	void update_menu();

	base::optional<Hover> get_hover(MultiViewWindow* win, const vec2& m) const;
};



#endif //MODEMESH_H
