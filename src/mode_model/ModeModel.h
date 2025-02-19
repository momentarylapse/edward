//
// Created by Michael Ankele on 2025-02-07.
//

#ifndef MODEMODEL_H
#define MODEMODEL_H


#include <lib/base/optional.h>
#include "../view/Mode.h"
#include "../view/Hover.h"
#include "data/DataModel.h"

class Light;
class MultiViewWindow;
class EntityPanel;
namespace xhui {
class Panel;
}
class Material;

class ModeModel : public Mode {
public:
	explicit ModeModel(Session* session);
	~ModeModel() override;

	void on_enter() override;
	void on_leave() override;
	void on_prepare_scene(const RenderParams& params) override;
	void on_draw_win(const RenderParams& params, MultiViewWindow* win) override;
	void on_draw_post(Painter*) override;

	void on_update_selection();

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



#endif //MODEMODEL_H
