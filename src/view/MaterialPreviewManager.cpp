//
// Created by michi on 3/15/26.
//

#include "MaterialPreviewManager.h"
#include <Session.h>
#include <lib/ygraphics/graphics-impl.h>
#include <lib/mesh/GeometryBall.h>
#include <lib/xhui/xhui.h>
#include <lib/yrenderer/scene/Light.h>
#include <lib/yrenderer/scene/path/RenderPathForward.h>
#include <lib/yrenderer/scene/MeshEmitter.h>
#include <lib/yrenderer/scene/RenderViewData.h>
#include <lib/yrenderer/target/HeadlessRendererVulkan.h>


MaterialPreviewManager::MaterialPreviewManager(Session* s) {
	session = s;
}

ygfx::Texture* MaterialPreviewManager::get_mat_texture(yrenderer::Material* m) {
	if (textures.contains(m))
		return textures[m].get();
	auto tex = new ygfx::Texture(PREVIEW_SIZE*2, PREVIEW_SIZE*2, "rgba:i8");
	textures.set(m, tex);
	dirty.add(m);
	if (!depth_buffer)
		depth_buffer = new ygfx::DepthBuffer(PREVIEW_SIZE*2, PREVIEW_SIZE*2, "d:f32");
	return tex;
}

class XEmitter : public yrenderer::MeshEmitter {
public:
	yrenderer::Material* material;
	owned<ygfx::VertexBuffer> vb;
	XEmitter(Session* s, yrenderer::Material* m) : yrenderer::MeshEmitter(s->ctx, "x") {
		material = m;
		vb = new ygfx::VertexBuffer("3f,3f,2f");
		auto g = GeometryBall::create({0,0,4}, 1, 32, 64);
		g.smoothen();
		g.build(vb.get());
	}
	void emit(const yrenderer::RenderParams &params, yrenderer::RenderViewData &rvd, bool shadow_pass) override {
		if (!material->cast_shadow and shadow_pass)
			return;

		auto shader = rvd.get_shader(material, 0, "default", "");
		if (shadow_pass)
			material = rvd.material_shadow;

		auto& rd = rvd.start(params, mat4::ID, shader, *material, 0, ygfx::PrimitiveTopology::TRIANGLES, vb.get());

		rd.draw_triangles(params, vb.get());
	}
};

string MaterialPreviewManager::get(yrenderer::Material *m) {
	auto tex = get_mat_texture(m);
	update();
	return xhui::texture_to_image(tex);
}

void MaterialPreviewManager::update() {
	if (dirty.num == 0)
		return;

	for (auto m: dirty) {
		auto tex = get_mat_texture(m);

		yrenderer::Light light;
		light.init(yrenderer::LightType::DIRECTIONAL, White);
		light._ang = quaternion::rotation({0.5f,-0.5f,0});
		light.harshness = 0.5f;
		light.allow_shadow = false;//true;
		light.enabled = true;

		yrenderer::RenderPathForward path(session->ctx, 256);
		path.background_color = color(0,0,0,0);
		path.add_opaque_emitter(new XEmitter(session, m));
		path.set_view({{0,0,0}, quaternion::ID, pi/6, 0.1f, 100});
		path.set_lights({&light});

		//	Image im(64, 64, Black);
		path.scene_view.cube_map = new ygfx::CubeMap(64, "rgba:i8");
		//	for (int i=0; i<6; i++)
		//		path.scene_view.cube_map->write_side(i, im);

		yrenderer::HeadlessRenderer hr(session->ctx, {tex, depth_buffer.get()});
		hr.add_child(&path);
		hr.render({1, false});
	}

	dirty.clear();
}

void MaterialPreviewManager::invalidate(yrenderer::Material* material) {
	dirty.add(material);
}
