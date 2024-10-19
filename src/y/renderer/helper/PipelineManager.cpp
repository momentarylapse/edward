/*
 * PipelineManager.cpp
 *
 *  Created on: 15 Dec 2021
 *      Author: michi
 */

#include "PipelineManager.h"
#ifdef USING_VULKAN
#include "../../graphics-impl.h"
#include "../../lib/base/map.h"
#include "../../lib/os/msg.h"

namespace PipelineManager {

struct AlphaPipelineKey {
	Shader* s;
	Alpha src, dst;
	vulkan::CullMode culling;
	bool write_z;
	bool operator==(const AlphaPipelineKey &o) const {
		return s == o.s and src == o.src and dst == o.dst and culling == o.culling and write_z == o.write_z;
	}
	bool operator>(const AlphaPipelineKey &o) const {
		if (s != o.s)
			return s > o.s;
		if (src != o.src)
			return src > o.src;
		if (dst != o.dst)
			return dst > o.dst;
		if (culling != o.culling)
			return culling > o.culling;
		if (write_z != o.write_z)
			return write_z > o.write_z;
		return false;
	}
};

static base::map<Shader*,GraphicsPipeline*> ob_pipelines;
static base::map<AlphaPipelineKey,GraphicsPipeline*> ob_pipelines_alpha;
static base::map<Shader*,GraphicsPipeline*> ob_pipelines_gui;

string topology2vk(PrimitiveTopology top) {
	if (top == PrimitiveTopology::POINTS)
		return "points";
	if (top == PrimitiveTopology::LINES)
		return "lines";
	if (top == PrimitiveTopology::LINESTRIP)
		return "line-strip";
	if (top == PrimitiveTopology::TRIANGLES)
		return "triangles";
	return "triangles";
}

GraphicsPipeline *get(Shader *s, RenderPass *rp, PrimitiveTopology top, VertexBuffer *vb) {
	if (ob_pipelines.contains(s))
		return ob_pipelines[s];
	msg_write("NEW PIPELINE");
	auto p = new GraphicsPipeline(s, rp, 0, topology2vk(top), vb);
	ob_pipelines.add({s, p});
	return p;
}
GraphicsPipeline *get_alpha(Shader *s, RenderPass *rp, PrimitiveTopology top, VertexBuffer *vb, Alpha src, Alpha dst, bool write_z, vulkan::CullMode culling) {
	AlphaPipelineKey key = {s, src, dst, culling, write_z};
	if (ob_pipelines_alpha.contains(key))
		return ob_pipelines_alpha[key];
	msg_write(format("NEW PIPELINE ALPHA %d %d", (int)src, (int)dst));
	auto p = new GraphicsPipeline(s, rp, 0, topology2vk(top), vb);
	if (!write_z)
		p->set_z(true, false);
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
	auto p = new GraphicsPipeline(s, rp, 0, "triangles", "3f,3f,2f");
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
