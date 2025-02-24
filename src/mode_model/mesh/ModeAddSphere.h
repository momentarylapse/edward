//
// Created by Michael Ankele on 2025-02-23.
//

#ifndef MODEADDSPHERE_H
#define MODEADDSPHERE_H


#include "../../view/Mode.h"
#include "../data/DataModel.h"
#include <data/mesh/PolygonMesh.h>

class MultiViewWindow;
class ModeMesh;

class ModeAddSphere : public Mode {
public:
	explicit ModeAddSphere(ModeMesh* parent);

	void on_enter() override;
	void on_leave() override;
	void on_prepare_scene(const RenderParams& params) override;
	void on_draw_win(const RenderParams& params, MultiViewWindow* win) override;
	void on_draw_post(Painter*) override;
	void on_key_down(int key) override;
	void on_left_button_down(const vec2&) override;
	void on_mouse_move(const vec2& m, const vec2& d) override;
	void update_mesh();

	ModeMesh* mode_mesh;
	Mode* get_parent() override;

	vec3 center;
	bool center_selected;
	float radius;
	PolygonMesh mesh;
	owned<VertexBuffer> vertex_buffer;

	enum class Type {
		Ball,
		Sphere
	} type;
	int slices[2];
	int complexity;
};



#endif //MODEADDSPHERE_H
