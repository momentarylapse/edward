//
// Created by michi on 7/27/26.
//

#ifndef EDWARD_MODEADDBONE_H
#define EDWARD_MODEADDBONE_H

#include <view/Mode.h>

class MultiViewWindow;
class ModeSkeleton;
class ModeMesh;

class ModeAddBone : public SubMode {
public:
	explicit ModeAddBone(ModeSkeleton* parent);

	void on_enter() override;
	void on_draw_post(Painter*) override;
	void on_key_down(int key) override;
	void on_left_button_down(const vec2&) override;

	ModeSkeleton* mode_skeleton;
	ModeMesh* mode_mesh;
};


#endif //EDWARD_MODEADDBONE_H
