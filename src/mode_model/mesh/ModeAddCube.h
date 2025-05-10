//
// Created by Michael Ankele on 2025-02-20.
//

#ifndef MODEADDCUBE_H
#define MODEADDCUBE_H


#include "../../view/Mode.h"
#include "../data/DataModel.h"
#include <data/mesh/PolygonMesh.h>

class MultiViewWindow;
class ModeMesh;

class ModeAddCube : public SubMode {
public:
	explicit ModeAddCube(ModeMesh* parent);

	void on_enter() override;
	void on_leave() override;
	void on_draw_win(const RenderParams& params, MultiViewWindow* win) override;
	void on_draw_post(Painter*) override;
	void on_key_down(int key) override;
	void on_left_button_down(const vec2&) override;
	void on_mouse_move(const vec2& m, const vec2& d) override;
	bool set_dpos3(const vec2& m);

	ModeMesh* mode_mesh;

	Array<vec3> points;
	vec3 length[3];
	PolygonMesh mesh;
	owned<VertexBuffer> vertex_buffer;

	int slices[3];
};



#endif //MODEADDCUBE_H
