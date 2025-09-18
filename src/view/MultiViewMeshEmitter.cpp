//
// Created by michi on 11.05.25.
//

#include "MultiViewMeshEmitter.h"
#include "MultiView.h"
#include "MultiViewWindow.h"
#include "Mode.h"
#include "DocumentSession.h"


void MultiViewBackgroundEmitter::emit(const yrenderer::RenderParams& params, yrenderer::RenderViewData& rvd, bool shadow_pass) {
	multi_view->doc->cur_mode->on_draw_background(params, rvd);
}

void MultiViewGeometryEmitter::emit(const yrenderer::RenderParams& params, yrenderer::RenderViewData& rvd, bool shadow_pass) {
	win->multi_view->doc->cur_mode->on_draw_win(params, win);
}

void MultiViewShadowGeometryEmitter::emit(const yrenderer::RenderParams& params, yrenderer::RenderViewData& rvd, bool shadow_pass) {
	multi_view->doc->cur_mode->on_draw_shadow(params, rvd);
}
