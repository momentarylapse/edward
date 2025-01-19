//
// Created by Michael Ankele on 2025-01-20.
//

#include "ModeWorld.h"

#include <Session.h>
#include <view/MultiView.h>

#include "data/DataWorld.h"
#include "data/WorldObject.h"
#include "data/WorldTerrain.h"
#include <y/renderer/Renderer.h>
#include <y/renderer/world/geometry/RenderViewData.h>
#include <y/renderer/world/geometry/SceneView.h>
#include <y/renderer/base.h>
#include <y/renderer/path/RenderPath.h>
#include <y/helper/ResourceManager.h>
#include <y/world/Material.h>
#include <y/world/Camera.h>
#include <y/world/Light.h>
#include <y/y/Entity.h>
#include <y/world/Model.h>
#include <y/world/ModelManager.h>
#include <y/world/Terrain.h>
#include <y/graphics-impl.h>

class DataWorldRenderer : public Renderer {
public:
	DataWorld* data_world;
	RenderViewData rvd;
	Light* light;
	base::map<Material*, ShaderCache> multi_pass_shader_cache[4];
	// material as id!
	Shader* get_shader(Material* material, int pass_no, const string& vertex_shader_module, const string& geometry_shader_module) {
		if (!multi_pass_shader_cache[pass_no].contains(material))
			multi_pass_shader_cache[pass_no].set(material, {});
		auto& cache = multi_pass_shader_cache[pass_no][material];
		RenderPathType type = RenderPathType::Forward;
	//	if (is_shadow_pass())
	//		cache._prepare_shader_multi_pass(type, *material_shadow, vertex_shader_module, geometry_shader_module, pass_no);
	//	else
			cache._prepare_shader_multi_pass(type, *material, vertex_shader_module, geometry_shader_module, pass_no);
		return cache.get_shader(type);
	}

	explicit DataWorldRenderer(DataWorld* w, SceneView* scene_view) : Renderer("world") {
		data_world = w;
		resource_manager = data_world->session->resource_manager;

		rvd.scene_view = scene_view;

		light = new Light(White, -1, -1);
		light->owner = new Entity;
		light->owner->ang = quaternion::rotation({1,0,0}, 0.5f);
		light->light.harshness = 0.5f;
	}
	void draw(const RenderParams& params) override {
		auto cb = params.command_buffer;
		cb->clear(params.area, {data_world->meta_data.background_color}, 1.0);

		/*if (cam_radius < 0) {
			vec3 vmin, vmax;
			data_world->get_bounding_box(vmin, vmax);
			cam_pos = (vmin + vmax) / 2;
			cam_radius = (vmax - vmin).length() * 0.7f;
		}*/

	//	scene_view.choose_lights();
		{
			rvd.scene_view->lights.clear();
			rvd.scene_view->shadow_index = -1;
			//	if (l->allow_shadow)
			//		scene_view.shadow_index = scene_view.lights.num;
			rvd.scene_view->lights.add(light);
		}

		rvd.scene_view->cam->update_matrices(params.desired_aspect_ratio);
		rvd.set_projection_matrix(rvd.scene_view->cam->m_projection);
		rvd.set_view_matrix(rvd.scene_view->cam->m_view);
		rvd.update_lights();
		rvd.ubo.num_lights = rvd.scene_view->lights.num;
		rvd.ubo.shadow_index = rvd.scene_view->shadow_index;

		rvd.begin_draw();

		for (auto& t: data_world->terrains) {
			t.terrain->prepare_draw(rvd.scene_view->cam->owner->pos);
			auto material = t.terrain->material.get();
			auto vb = t.terrain->vertex_buffer.get();

			auto shader = get_shader(material, 0, t.terrain->vertex_shader_module, "");
			auto& rd = rvd.start(params,  mat4::translation(t.pos), shader, *material, 0, PrimitiveTopology::TRIANGLES, vb);
			cb->push_constant(0, 4, &t.terrain->texture_scale[0].x);
			cb->push_constant(4, 4, &t.terrain->texture_scale[1].x);
			rd.apply(params);
			cb->draw(vb);
		}

		for (auto& o: data_world->objects) {
			for (int k=0; k<o.object->mesh[0]->sub.num; k++) {
				auto m = o.object;
				auto material = m->material[k];
				auto vb = m->mesh[0]->sub[k].vertex_buffer;

				auto shader = get_shader(material, 0, m->_template->vertex_shader_module, "");
				auto& rd = rvd.start(params,  mat4::translation(o.pos) * mat4::rotation(o.ang), shader, *material, 0, PrimitiveTopology::TRIANGLES, vb);
				rd.apply(params);
				cb->draw(vb);
			}
		}
	}
};

ModeWorld::ModeWorld(Session* session) : Mode(session) {
	multi_view = new MultiView;
	data = new DataWorld(session);
}


Renderer* ModeWorld::create_renderer(SceneView* scene_view) {
	return new DataWorldRenderer(data, scene_view);
}

