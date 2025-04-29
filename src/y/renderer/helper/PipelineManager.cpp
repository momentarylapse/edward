/*
 * PipelineManager.cpp
 *
 *  Created on: 15 Dec 2021
 *      Author: michi
 */

#include "PipelineManager.h"
#ifdef USING_VULKAN
#include "../../graphics-impl.h"
#include <lib/base/map.h>
#include <lib/os/msg.h>

namespace PipelineManager {

struct PipelineKey {
	Shader* s;
	int param;
	bool operator==(const PipelineKey &o) const {
		return s == o.s and param == o.param;
	}
	bool operator>(const PipelineKey &o) const {
		if (s != o.s)
			return s > o.s;
		if (param != o.param)
			return param > o.param;
		return false;
	}
};

static base::map<PipelineKey,GraphicsPipeline*> ob_pipelines;
static base::map<PipelineKey,GraphicsPipeline*> ob_pipelines_alpha;
static base::map<Shader*,GraphicsPipeline*> ob_pipelines_gui;

GraphicsPipeline *get(Shader *s, RenderPass *rp, PrimitiveTopology top, VertexBuffer *vb, vulkan::CullMode culling, bool test_z, bool write_z) {
	PipelineKey key = {s, (int)culling + ((int)write_z << 4) + ((int)test_z << 5)};
	if (ob_pipelines.contains(key))
		return ob_pipelines[key];
	msg_write("NEW PIPELINE");
	auto p = new GraphicsPipeline(s, rp, 0, top, vb);
	p->set_culling(culling);
	p->set_z(test_z, write_z);
	p->rebuild();
	ob_pipelines.add({key, p});
	return p;
}
GraphicsPipeline *get_alpha(Shader *s, RenderPass *rp, PrimitiveTopology top, VertexBuffer *vb, Alpha src, Alpha dst, vulkan::CullMode culling, bool test_z, bool write_z) {
	PipelineKey key = {s, (int)src + ((int)dst << 4) + ((int)culling << 8) + ((int)write_z << 12) + ((int)test_z << 13)};
	if (ob_pipelines_alpha.contains(key))
		return ob_pipelines_alpha[key];
	msg_write(format("NEW PIPELINE ALPHA %d %d", (int)src, (int)dst));
	auto p = new GraphicsPipeline(s, rp, 0, top, vb);
	p->set_z(test_z, write_z);
	p->set_blend(src, dst);
	p->set_culling(culling);
	p->rebuild();
	ob_pipelines_alpha.add({key, p});
	return p;
}

GraphicsPipeline *get_gui(Shader *s, RenderPass *rp, const string &format) {
	if (ob_pipelines_gui.contains(s))
		return ob_pipelines_gui[s];
	msg_write("NEW PIPELINE GUI");
	auto p = new GraphicsPipeline(s, rp, 0, PrimitiveTopology::TRIANGLES, "3f,3f,2f");
	p->set_blend(Alpha::SOURCE_ALPHA, Alpha::SOURCE_INV_ALPHA);
	p->set_z(false, false);
	p->rebuild();
	ob_pipelines_gui.add({s, p});
	return p;
}

void clear() {
	ob_pipelines.clear();
	ob_pipelines_alpha.clear();
	ob_pipelines_gui.clear();
}


}

#endif
