/*
 * PostProcessor.cpp
 *
 *  Created on: Dec 13, 2021
 *      Author: michi
 */

#include "PostProcessor.h"


string callable_name(const void *c);

PostProcessorStage::PostProcessorStage(const string &name) : Renderer(name) {
}


PostProcessorStageUser::PostProcessorStageUser(const PostProcessorStageUser::Callback *p, const PostProcessorStageUser::Callback *d) :
		PostProcessorStage(callable_name(d)) {
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

PostProcessor::PostProcessor() : Renderer("post") {
}

PostProcessor::~PostProcessor() {
}


void PostProcessor::add_stage(const PostProcessorStageUser::Callback *p, const PostProcessorStageUser::Callback *d) {
	stages.add(new PostProcessorStageUser(p, d));
	rebuild();
}
void PostProcessor::reset() {
	stages.clear();
	//if (hdr)
	//	stages.add(hdr);
	rebuild();
}

void PostProcessor::rebuild() {
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

void PostProcessor::set_hdr(PostProcessorStage *_hdr) {
	throw Exception("PostProcessor.set_hdr... nope");
	hdr = _hdr;
	rebuild();
}
