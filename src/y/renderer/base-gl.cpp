#include "base.h"
#include "../y/EngineData.h"
#include "../helper/ResourceManager.h"
#include "../graphics-impl.h"
#include <lib/os/msg.h>
#if __has_include(<lib/xhui/Painter.h>)
#include <lib/xhui/Painter.h>
#define HAS_XHUI
#endif

#ifdef USING_OPENGL


namespace nix {
extern bool allow_separate_vertex_arrays;
}

#ifdef HAS_XHUI
namespace xhui {
extern owned<nix::Context> _nix_context;
}
#endif


Context* api_init(GLFWwindow* window) {
	nix::allow_separate_vertex_arrays = true;
	nix::default_shader_bindings = false;
	auto gl = nix::init();

	if (gl->total_mem() > 0) {
		msg_write(format("VRAM: %d mb  of  %d mb available", gl->available_mem() / 1024, gl->total_mem() / 1024));
	}

	nix::create_query_pool(MAX_TIMESTAMP_QUERIES);

	_create_default_textures();

	return gl;
}

Context* api_init_xhui(xhui::Painter* p) {
#ifdef HAS_XHUI
	nix::create_query_pool(MAX_TIMESTAMP_QUERIES);
	_create_default_textures();
	return xhui::_nix_context.get();
#else
	return nullptr;
#endif
}

void reset_gpu_timestamp_queries() {
	gpu_timestamp_queries.clear();
	gpu_timestamp_queries.simple_reserve(256);
}

void gpu_timestamp(const RenderParams&, int channel) {
	if (gpu_timestamp_queries.num >= MAX_TIMESTAMP_QUERIES)
		return;
	nix::query_timestamp(gpu_timestamp_queries.num);
	gpu_timestamp_queries.add(channel);
}

void gpu_timestamp_begin(const RenderParams& params, int channel) {
	gpu_timestamp(params, channel);
}

void gpu_timestamp_end(const RenderParams& params, int channel) {
	gpu_timestamp(params, channel | (int)0x80000000);
}

Array<float> gpu_read_timestamps() {
	auto tt = nix::get_timestamps(0, gpu_timestamp_queries.num);
	Array<float> result;
	result.resize(tt.num);
	for (int i=0; i<tt.num; i++)
		result[i] = (float)(tt[i] - tt[0]) * 1e-9f;
	return result;
}

void gpu_flush() {
	nix::flush();
}

void api_end() {
}

#endif
