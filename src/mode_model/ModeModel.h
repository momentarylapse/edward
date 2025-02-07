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

	DataModel* data;
	VertexBuffer* vertex_buffer;
	Material* material;
	Array<int> event_ids;
};



#endif //MODEMODEL_H
