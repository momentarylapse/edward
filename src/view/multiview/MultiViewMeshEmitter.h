//
// Created by michi on 11.05.25.
//

#ifndef MULTIVIEWMESHEMITTER_H
#define MULTIVIEWMESHEMITTER_H

#include <lib/yrenderer/scene/MeshEmitter.h>

#include "MultiView.h"

class MultiViewWindow;
class MultiView;

class MultiViewBackgroundEmitter : public yrenderer::MeshEmitter {
public:
	MultiView* multi_view;
	explicit MultiViewBackgroundEmitter(MultiView* mv) : yrenderer::MeshEmitter(mv->ctx, "geo") {
		multi_view = mv;
	}
	void emit(const yrenderer::RenderParams& params, yrenderer::RenderViewData& rvd, bool shadow_pass) override;
};

class MultiViewGeometryEmitter : public yrenderer::MeshEmitter {
public:
	MultiViewWindow* win;
	explicit MultiViewGeometryEmitter(MultiViewWindow* _win) : MeshEmitter(_win->multi_view->ctx, "geo") {
		win = _win;
	}
	void emit(const yrenderer::RenderParams& params, yrenderer::RenderViewData& rvd, bool shadow_pass) override;
};

class MultiViewShadowGeometryEmitter : public yrenderer::MeshEmitter {
public:
	MultiView* multi_view;
	explicit MultiViewShadowGeometryEmitter(MultiView* mv) : MeshEmitter(mv->ctx, "geo-shdw") {
		multi_view = mv;
	}
	void emit(const yrenderer::RenderParams& params, yrenderer::RenderViewData& rvd, bool shadow_pass) override;
};


#endif //MULTIVIEWMESHEMITTER_H
