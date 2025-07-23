//
// Created by Michael Ankele on 2025-05-06.
//

#include "WorldModelsEmitter.h"
#include <lib/profiler/Profiler.h>
#include <renderer/base.h>
#include <renderer/scene/RenderViewData.h>
#include <renderer/scene/SceneView.h>
#include <world/Model.h>
#include <world/ModelManager.h>
#include <world/components/Animator.h>
#include <world/Camera.h>
#include <y/ComponentManager.h>
#include <y/Entity.h>
#include <graphics-impl.h>
#include <lib/base/sort.h>

WorldModelsEmitter::WorldModelsEmitter() : MeshEmitter("mod") {
}


void WorldModelsEmitter::emit(const RenderParams& params, RenderViewData& rvd, bool shadow_pass) {
	profiler::begin(channel);
	gpu_timestamp_begin(params, channel);

	auto& list = ComponentManager::get_list_family<Model>();

	for (auto m: list) {
		auto ani = m->owner ? m->owner->get_component<Animator>() : nullptr;
		for (int i=0; i<m->material.num; i++) {
			auto material = m->material[i];
			if (material->is_transparent())
				continue;
			if (!material->cast_shadow and shadow_pass)
				continue;

			auto shader = rvd.get_shader(material, 0, m->_template->vertex_shader_module, "");
			if (shadow_pass)
				material = rvd.material_shadow;

			m->update_matrix();
			auto vb = m->mesh[0]->sub[i].vertex_buffer;
			auto& rd = rvd.start(params, m->_matrix, shader, *material, 0, PrimitiveTopology::TRIANGLES, vb);

			if (ani) {
				ani->buf->update_array(ani->dmatrix);
#ifdef USING_VULKAN
				rd.dset->set_uniform_buffer(BINDING_BONE_MATRICES, ani->buf);
#else
				nix::bind_uniform_buffer(BINDING_BONE_MATRICES, ani->buf);
#endif
			}

			rd.draw_triangles(params, vb);
		}
	}
	gpu_timestamp_end(params, channel);
	profiler::end(channel);
}

void WorldModelsEmitter::emit_transparent(const RenderParams& params, RenderViewData& rvd) {
	profiler::begin(channel);
	gpu_timestamp_begin(params, channel);
	auto cam = rvd.scene_view->cam;

	struct DrawCallData {
		Model* model;
		int material_index;
		float z;
	};
	Array<DrawCallData> draw_calls;

	auto& list = ComponentManager::get_list_family<Model>();

	for (auto m: list) {
		for (int i=0; i<m->material.num; i++) {
			auto material = m->material[i];
			if (!material->is_transparent())
				continue;

			draw_calls.add({m, i, (m->owner->pos - cam->owner->pos).length()});
		}
	}

	// sort: far to near
	draw_calls = base::sorted(draw_calls, [] (const auto& a, const auto& b) { return a.z >= b.z; });


	// draw!
	for (const auto& dc: draw_calls) {
		auto m = dc.model;
		auto material = dc.model->material[dc.material_index];
		int i = dc.material_index;
		auto ani = m->owner ? m->owner->get_component<Animator>() : nullptr;

		m->update_matrix();
		auto vb = m->mesh[0]->sub[i].vertex_buffer;

		for (int k=0; k<material->num_passes; k++) {
			auto shader = rvd.get_shader(material, k, m->_template->vertex_shader_module, "");

			auto& rd = rvd.start(params, m->_matrix, shader, *material, k, PrimitiveTopology::TRIANGLES, vb);

			if (ani) {
				ani->buf->update_array(ani->dmatrix);
#ifdef USING_VULKAN
				rd.dset->set_uniform_buffer(BINDING_BONE_MATRICES, ani->buf);
#else
				nix::bind_uniform_buffer(BINDING_BONE_MATRICES, ani->buf);
#endif
			}

			rd.draw_triangles(params, vb);
		}
	}
	gpu_timestamp_end(params, channel);
	profiler::end(channel);
}


