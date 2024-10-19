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
void api_end();

extern Texture *tex_white;
extern Texture *tex_black;

void gpu_flush();

void reset_gpu_timestamp_queries();
#ifdef USING_VULKAN
void gpu_timestamp(CommandBuffer *cb, int channel);
void gpu_timestamp_begin(CommandBuffer *cb, int channel);
void gpu_timestamp_end(CommandBuffer *cb, int channel);
#else
void gpu_timestamp(int channel);
void gpu_timestamp_begin(int channel);
void gpu_timestamp_end(int channel);
#endif
Array<float> gpu_read_timestamps();
extern Array<int> gpu_timestamp_queries;

#ifdef USING_VULKAN
extern vulkan::DescriptorPool *pool;
extern vulkan::Device *device;
#endif
