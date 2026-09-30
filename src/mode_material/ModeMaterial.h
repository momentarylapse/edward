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

class LightController {
public:
	int hover(MultiViewWindow* win, const vec2& m) const;
	void draw(const yrenderer::RenderParams& params, MultiViewWindow* win);
	void on_left_button_down(MultiViewWindow* win, const vec2& m);
	void on_mouse_move(MultiViewWindow* win, const vec2& m);
	void on_left_button_up(MultiViewWindow* win, const vec2& m);
	MultiView* multi_view = nullptr;
	int hovering = -1;
	int dragging = -1;
	bool dragging_front = false;
};

class ModeMaterial : public Mode {
public:
	explicit ModeMaterial(DocumentSession* session);
	~ModeMaterial() override;

	void on_enter() override;
	void on_leave() override;
	void on_connect_events() override;
	void on_enter_rec() override;
	void on_leave_rec() override;

	void on_key_down(int key) override;
	void on_mouse_move(const vec2 &m, const vec2 &d) override;
	void on_left_button_down(const vec2 &) override;
	void on_left_button_up(const vec2 &) override;

	void on_prepare_scene(const yrenderer::RenderParams& params) override;
	void on_draw_background(const yrenderer::RenderParams& params, yrenderer::RenderViewData& rvd) override;
	void on_draw_win(const yrenderer::RenderParams& params, MultiViewWindow* win) override;
	void on_draw_shadow(const yrenderer::RenderParams& params, yrenderer::RenderViewData& rvd) override;

	void on_draw_post(Painter*) override;
	void optimize_view();

	void on_command(const string& id) override;

	DataMaterial* data;
	owned<ygfx::VertexBuffer> vertex_buffer;
	PreviewMesh preview_mesh;
	const yrenderer::Material* material;

	owned<ygfx::VertexBuffer> vertex_buffer_ground;
	owned<yrenderer::Material> material_ground;

	owned<yrenderer::Light> spot_light;
	int hover = -1;

	LightController light_controller;

	void set_mesh(PreviewMesh m);
};



#endif //MODEMATERIAL_H
