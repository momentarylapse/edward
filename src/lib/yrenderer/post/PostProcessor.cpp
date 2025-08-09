/*
 * PostProcessor.cpp
 *
 *  Created on: Dec 13, 2021
 *      Author: michi
 */

#include "PostProcessor.h"

namespace yrenderer {

PostProcessorStage::PostProcessorStage(Context* ctx, const string &name) : Renderer(ctx, name) {
}


PostProcessorStageUser::PostProcessorStageUser(Context* ctx, const string& name, const Callback *p, const Callback *d) :
		PostProcessorStage(ctx, name) {
	func_prepare = p;
	func_draw = d;
}

void PostProcessorStageUser::prepare(const RenderParams& params) {
	if (func_prepare)
		(*func_prepare)(params);
}
void PostProcessorStageUser::draw(const RenderParams& params) {
	if (func_draw)
		(*func_draw)(params);
}

PostProcessorBase::PostProcessorBase(Context* ctx) : Renderer(ctx, "post") {
}

PostProcessorBase::~PostProcessorBase() {
}


void PostProcessorBase::add_stage(const string& name, const PostProcessorStageUser::Callback *p, const PostProcessorStageUser::Callback *d) {
	stages.add(new PostProcessorStageUser(ctx, name, p, d));
	rebuild();
}
void PostProcessorBase::reset() {
	stages.clear();
	//if (hdr)
	//	stages.add(hdr);
	rebuild();
}

void PostProcessorBase::rebuild() {
#if 0
	auto stages_eff = stages;
	//if (hdr)
	//	stages_eff.insert(hdr, 0);

	if (stages_eff.num > 0) {
		stages_eff[0]->children = children;
		parent->children = {stages_eff.back()};
	} else {
		parent->children = children;
	}
	for (int i=1; i<stages_eff.num; i++)
		stages_eff[i]->children = {stages_eff[i-1]};
	for (int i=0; i<stages_eff.num-1; i++)
		stages_eff[i]->parent = stages_eff[i+1];
#endif
}

void PostProcessorBase::set_hdr(PostProcessorStage *_hdr) {
	throw Exception("PostProcessor.set_hdr... nope");
	hdr = _hdr;
	rebuild();
}

}
