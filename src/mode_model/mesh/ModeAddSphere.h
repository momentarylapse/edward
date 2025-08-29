//
// Created by Michael Ankele on 2025-02-23.
//

#ifndef MODEADDSPHERE_H
#define MODEADDSPHERE_H


#include "../../view/Mode.h"
#include "../data/DataModel.h"
#include <lib/mesh/PolygonMesh.h>
#include <lib/base/optional.h>

class MultiViewWindow;
class ModeMesh;

class ModeAddSphere : public SubMode {
public:
	explicit ModeAddSphere(ModeMesh* parent);

	void on_enter() override;
	void on_leave() override;
	void on_draw_win(const yrenderer::RenderParams& params, MultiViewWindow* win) override;
	void on_draw_post(Painter*) override;
	void on_key_down(int key) override;
	void on_left_button_down(const vec2&) override;
	void on_mouse_move(const vec2& m, const vec2& d) override;
	void update_mesh();

	ModeMesh* mode_mesh;

	base::optional<vec3> center;
	vec3 next_point;
	float radius;
	PolygonMesh mesh;
	owned<ygfx::VertexBuffer> vertex_buffer;

	enum class Type {
		Ball,
		Sphere
	} type;
	int slices[2];
	int complexity;
};



#endif //MODEADDSPHERE_H
