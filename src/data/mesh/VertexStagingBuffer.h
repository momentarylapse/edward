//
// Created by Michael Ankele on 2025-02-19.
//

#ifndef VERTEXSTAGINGBUFFER_H
#define VERTEXSTAGINGBUFFER_H

#include <y/world/Material.h>

class vec3;

class VertexStagingBuffer {
public:
	Array<vec3> p, n;
	Array<float> uv[MATERIAL_MAX_TEXTURES];
	void build(VertexBuffer *vb, int num_textures);
	void clear();
};


#endif //VERTEXSTAGINGBUFFER_H
