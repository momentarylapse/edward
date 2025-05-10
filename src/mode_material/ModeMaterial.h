//
// Created by Michael Ankele on 2025-04-30.
//

#ifndef MODEMATERIAL_H
#define MODEMATERIAL_H


#include <lib/base/optional.h>
#include "../view/Mode.h"
#include "data/DataMaterial.h"

class Light;
class MultiViewWindow;
namespace xhui {
	class Panel;
}

enum class PreviewMesh {
	CUBE,
	ICOSAHEDRON,
	SPHERE,
	TORUS,
	TORUS_KNOT,
	TEAPOT
};

class ModeMaterial : public Mode {
public:
	explicit ModeMaterial(Session* session);

	void on_enter() override;
	void on_leave() override;

	void on_key_down(int key) override;

	void on_prepare_scene(const RenderParams& params) override;
	void on_draw_background(const RenderParams& params, RenderViewData& rvd) override;
	void on_draw_win(const RenderParams& params, MultiViewWindow* win) override;
	void on_draw_shadow(const RenderParams& params, RenderViewData& rvd) override;

	void on_draw_post(Painter*) override;
	void optimize_view();

	void on_command(const string& id) override;

	DataMaterial* data;
	owned<VertexBuffer> vertex_buffer;
	PreviewMesh preview_mesh;
	owned<Material> material;

	owned<VertexBuffer> vertex_buffer_ground;
	owned<Material> material_ground;

	Light* spot_light = nullptr;

	void set_mesh(PreviewMesh m);
};



#endif //MODEMATERIAL_H
