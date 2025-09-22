//
// Created by Michael Ankele on 2025-04-30.
//

#ifndef MODEMATERIAL_H
#define MODEMATERIAL_H


#include <lib/base/optional.h>
#include "../view/Mode.h"
#include "data/DataMaterial.h"

namespace yrenderer {
	class Light;
}
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
	explicit ModeMaterial(DocumentSession* session);

	void on_enter() override;
	void on_leave() override;

	void on_key_down(int key) override;

	void on_prepare_scene(const yrenderer::RenderParams& params) override;
	void on_draw_background(const yrenderer::RenderParams& params, yrenderer::RenderViewData& rvd) override;
	void on_draw_win(const yrenderer::RenderParams& params, MultiViewWindow* win) override;
	void on_draw_shadow(const yrenderer::RenderParams& params, yrenderer::RenderViewData& rvd) override;

	void on_draw_post(Painter*) override;
	void optimize_view();

	void on_command(const string& id) override;
	void on_set_menu() override;

	DataMaterial* data;
	owned<ygfx::VertexBuffer> vertex_buffer;
	PreviewMesh preview_mesh;
	owned<yrenderer::Material> material;

	owned<ygfx::VertexBuffer> vertex_buffer_ground;
	owned<yrenderer::Material> material_ground;

	owned<yrenderer::Light> spot_light;

	void set_mesh(PreviewMesh m);
};



#endif //MODEMATERIAL_H
