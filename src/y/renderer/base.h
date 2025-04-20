/*
 * base.h
 *
 *  Created on: 21 Nov 2021
 *      Author: michi
 */


#pragma once

#include "Renderer.h"
#include "../graphics-fwd.h"

struct GLFWwindow;

Context* api_init(GLFWwindow* window);
#ifdef USING_VULKAN
Context* api_init_external(vulkan::Instance* instance, vulkan::Device* device);
#endif
void api_end();
void _create_default_textures();

extern Texture *tex_white;
extern Texture *tex_black;

void gpu_flush();

static constexpr int MAX_TIMESTAMP_QUERIES = 4096;

void reset_gpu_timestamp_queries();
void gpu_timestamp(const RenderParams& params, int channel);
void gpu_timestamp_begin(const RenderParams& params, int channel);
void gpu_timestamp_end(const RenderParams& params, int channel);
Array<float> gpu_read_timestamps();
extern Array<int> gpu_timestamp_queries;

#ifdef USING_VULKAN
extern vulkan::DescriptorPool *pool;
extern vulkan::Device *device;
#endif
