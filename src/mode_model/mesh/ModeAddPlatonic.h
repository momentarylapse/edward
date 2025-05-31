//
// Created by Michael Ankele on 2025-02-24.
//

#ifndef MODEADDPLATONIC_H
#define MODEADDPLATONIC_H


#include "../../view/Mode.h"
#include "../data/DataModel.h"
#include <data/mesh/PolygonMesh.h>

class MultiViewWindow;
class ModeMesh;

class ModeAddPlatonic : public SubMode {
public:
	explicit ModeAddPlatonic(ModeMesh* parent);

	void on_enter() override;
	void on_leave() override;
	void on_draw_win(const RenderParams& params, MultiViewWindow* win) override;
	void on_draw_post(Painter*) override;
	void on_key_down(int key) override;
	void on_left_button_down(const vec2&) override;
	void on_mouse_move(const vec2& m, const vec2& d) override;
	void update_mesh();

	ModeMesh* mode_mesh;

	vec3 center;
	bool center_selected;
	vec3 point2;
	float radius;
	PolygonMesh mesh;
	owned<VertexBuffer> vertex_buffer;

	enum class Type {
		Tetrahedron,
		Cube,
		Octahedron,
		Dodecahedron,
		Icosahedron,
		Teapot
	} type;
	int complexity;
};



#endif //MODEADDPLATONIC_H
