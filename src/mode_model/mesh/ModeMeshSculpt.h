//
// Created by Michael Ankele on 2025-03-03.
//

#ifndef MODEMESHSCULPT_H
#define MODEMESHSCULPT_H


#include "../../view/Mode.h"
#include "../data/DataModel.h"
#include <data/mesh/PolygonMesh.h>
#include <lib/base/optional.h>

class MultiViewWindow;
class ModeMesh;

class ModeMeshSculpt : public Mode {
public:
	explicit ModeMeshSculpt(ModeMesh* parent);

	void on_enter() override;
	void on_leave() override;
	void on_prepare_scene(const RenderParams& params) override;
	void on_draw_win(const RenderParams& params, MultiViewWindow* win) override;
	void on_draw_post(Painter*) override;
	void on_key_down(int key) override;
	void on_left_button_down(const vec2&) override;
	void on_mouse_move(const vec2& m, const vec2& d) override;

	ModeMesh* mode_mesh;
	Mode* get_parent() override;

	struct Brush {
		float radius;
		float depth;
	};
	Brush brush;

	struct Target {
		vec3 pos;
		vec3 n;
	};
	base::optional<Target> target;
	DataModel* data;
};


#endif //MODEMESHSCULPT_H
