//
// Created by michi on 11.05.25.
//

#include "MultiViewMeshEmitter.h"
#include "MultiView.h"
#include "MultiViewWindow.h"
#include "Mode.h"
#include <Session.h>


void MultiViewBackgroundEmitter::emit(const RenderParams& params, RenderViewData& rvd, bool shadow_pass) {
	multi_view->session->cur_mode->on_draw_background(params, rvd);
}

void MultiViewGeometryEmitter::emit(const RenderParams& params, RenderViewData& rvd, bool shadow_pass) {
	win->multi_view->session->cur_mode->on_draw_win(params, win);
}

void MultiViewShadowGeometryEmitter::emit(const RenderParams& params, RenderViewData& rvd, bool shadow_pass) {
	multi_view->session->cur_mode->on_draw_shadow(params, rvd);
}
