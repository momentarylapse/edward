//
// Created by Michael Ankele on 2025-04-14.
//

#ifndef MODEADDFROMLATHE_H
#define MODEADDFROMLATHE_H

#include "../../view/Mode.h"
#include "../data/DataModel.h"
#include <data/mesh/PolygonMesh.h>
#include <lib/base/optional.h>

class MultiViewWindow;
class ModeMesh;

class ModeAddFromLathe : public SubMode {
public:
	explicit ModeAddFromLathe(ModeMesh* parent);

	void on_enter() override;
	void on_leave() override;
	void on_draw_win(const RenderParams& params, MultiViewWindow* win) override;
	void on_draw_post(Painter*) override;
	void on_key_down(int key) override;
	void on_left_button_down(const vec2&) override;
	void on_mouse_move(const vec2& m, const vec2& d) override;

	vec3 suggest_axis(const vec3& p2) const;
	void create_turned_mesh();

	ModeMesh* mode_mesh;

	base::optional<vec3> center;
	base::optional<vec3> axis;
	vec3 preview_axis;
	Array<vec3> contour;
	vec3 next_point;
	PolygonMesh mesh;
	owned<VertexBuffer> vertex_buffer;

	int slices;
};
#endif //MODEADDFROMLATHE_H
