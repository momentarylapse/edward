//
// Created by Michael Ankele on 2025-05-13.
//

#ifndef MODEADDCYLINDER_H
#define MODEADDCYLINDER_H

#include "../../view/Mode.h"
#include "../data/DataModel.h"
#include <data/mesh/PolygonMesh.h>

class MultiViewWindow;
class ModeMesh;

class ModeAddCylinder : public SubMode {
public:
	explicit ModeAddCylinder(ModeMesh* parent);

	void on_enter() override;
	void on_leave() override;
	void on_draw_win(const RenderParams& params, MultiViewWindow* win) override;
	void on_draw_post(Painter*) override;
	void on_key_down(int key) override;
	void on_left_button_down(const vec2&) override;
	void on_mouse_move(const vec2& m, const vec2& d) override;

	ModeMesh* mode_mesh;

	Array<vec3> points;
	PolygonMesh mesh;
	owned<VertexBuffer> vertex_buffer;

	vec3 next_point;

	int rings, edges;
	bool round;
};


#endif //MODEADDCYLINDER_H
