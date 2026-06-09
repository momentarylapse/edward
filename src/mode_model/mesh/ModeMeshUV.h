#pragma once



#include <view/Mode.h>
#include "../data/DataModel.h"
#include "../data/ModelMesh.h"

class MultiViewWindow;
class ModeMesh;

class ModeMeshUV : public SubMode {
public:
	explicit ModeMeshUV(ModeMesh* parent);

	void on_enter() override;
	void on_enter_rec() override;
	void on_leave_rec() override;
	void on_draw_win(const yrenderer::RenderParams& params, MultiViewWindow* win) override;
	void on_draw_post(Painter*) override;
	void on_draw_background(const yrenderer::RenderParams &params, yrenderer::RenderViewData &rvd) override;

	base::optional<Hover> get_hover(MultiViewWindow* win, const vec2& m) const;
	Selection select_in_rect(MultiViewWindow* win, const rect& r);
	base::optional<Box> get_selection_box(const Selection& sel) const;

	void update_uvs();
	void import_selection();
	void export_selection();

	ModeMesh* mode_mesh;
	Array<ModelSkinVertexDummy> uv;
	owned<ygfx::VertexBuffer> vb_bg;
	yrenderer::Material* material_bg = nullptr;
};

