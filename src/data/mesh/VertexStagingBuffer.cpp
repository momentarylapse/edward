//
// Created by Michael Ankele on 2025-02-19.
//

#include "VertexStagingBuffer.h"
#include <y/graphics-impl.h>
#include <lib/math/plane.h>
#include <lib/math/vec2.h>


void VertexStagingBuffer::build(VertexBuffer *vb, int num_textures) {
	Array<float> temp;
	int d = 3 + 3 + 2*num_textures;
	temp.resize(p.num * d);
	for (int i=0; i<p.num; i++) {
		*(vec3*)&temp[i * d] = p[i];
		*(vec3*)&temp[i * d + 3] = n[i];
		for (int l=0; l<num_textures; l++)
			*(vec2*)&temp[i * d + 6 + l*2] = *(vec2*)&uv[l][2*i];
	}
	vb->update(temp);
#ifdef USING_VULKAN
	vb->vertex_count = p.num;
	vb->output_count = p.num;
#endif

	/*vb->update(0, p);
	vb->update(1, n);
	for (int l=0; l<num_textures; l++)
		vb->update(l+2, uv[l]);*/
}

void VertexStagingBuffer::clear() {
	p.clear();
	n.clear();
	for (int i=0; i<MATERIAL_MAX_TEXTURES; i++)
		uv[i].clear();
}


