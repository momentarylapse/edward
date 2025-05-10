//
// Created by Michael Ankele on 2025-02-08.
//

#ifndef MODEADDVERTEX_H
#define MODEADDVERTEX_H


#include "../../view/Mode.h"
#include "../data/DataModel.h"

class MultiViewWindow;
class ModeMesh;

class ModeAddVertex : public SubMode {
public:
	explicit ModeAddVertex(ModeMesh* parent);

	void on_enter() override;
	void on_draw_post(Painter*) override;
	void on_key_down(int key) override;
	void on_left_button_down(const vec2&) override;

	ModeMesh* mode_mesh;
};



#endif //MODEADDVERTEX_H
