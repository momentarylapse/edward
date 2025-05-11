//
// Created by michi on 11.05.25.
//

#ifndef MULTIVIEWMESHEMITTER_H
#define MULTIVIEWMESHEMITTER_H

#include <renderer/scene/MeshEmitter.h>

class MultiViewWindow;
class MultiView;

class MultiViewBackgroundEmitter : public MeshEmitter {
public:
	MultiView* multi_view;
	explicit MultiViewBackgroundEmitter(MultiView* mv) : MeshEmitter("geo") {
		multi_view = mv;
	}
	void emit(const RenderParams& params, RenderViewData& rvd, bool shadow_pass) override;
};

class MultiViewGeometryEmitter : public MeshEmitter {
public:
	MultiViewWindow* win;
	explicit MultiViewGeometryEmitter(MultiViewWindow* _win) : MeshEmitter("geo") {
		win = _win;
	}
	void emit(const RenderParams& params, RenderViewData& rvd, bool shadow_pass) override;
};

class MultiViewShadowGeometryEmitter : public MeshEmitter {
public:
	MultiView* multi_view;
	explicit MultiViewShadowGeometryEmitter(MultiView* mv) : MeshEmitter("geo-shdw") {
		multi_view = mv;
	}
	void emit(const RenderParams& params, RenderViewData& rvd, bool shadow_pass) override;
};


#endif //MULTIVIEWMESHEMITTER_H
