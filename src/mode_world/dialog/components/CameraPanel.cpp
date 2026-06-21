//
// Created by michi on 9/30/25.
//

#include "CameraPanel.h"
#include "../../data/DataWorld.h"
#include <y/world/components/Camera.h>
#include <y/ecs/EntityManager.h>
#include <y/world/Model.h>
#include <y/world/Terrain.h>
#include <y/world/components/Light.h>
#include <y/ecs/Entity.h>
#include <stuff/PluginManager.h>
#include <cmath>
#include <view/DrawingHelper.h>
#include <lib/base/iter.h>
#include <lib/os/msg.h>
#include <lib/xhui/xhui.h>
#include <lib/ygraphics/graphics-impl.h>
#include <lib/yrenderer/scene/path/RenderPathForward.h>
#include <lib/yrenderer/target/XhuiRenderer.h>
#include <lib/yrenderer/post/HDRResolver.h>

class CPEmitter : public yrenderer::MeshEmitter {
public:
	DataWorld* data;
	CPEmitter(DataWorld* _data) : MeshEmitter(_data->session->ctx, "xxx") {
		data = _data;
	}
	void emit(const yrenderer::RenderParams &params, yrenderer::RenderViewData &rvd, bool shadow_pass) override {

#ifdef USING_VULKAN
		auto cb = params.command_buffer;
#endif

		const auto terrains = data->entity_manager->get_component_list<TerrainRef>();
		for (auto tr: terrains)
			if (auto t = tr->terrain) {
				t->prepare_draw(tr->owner, rvd.scene_view->main_camera_params.pos);
				auto material = tr->material;
				if (!material)
					material = data->session->resource_manager->load_material("");
				auto vb = t->vertex_buffer.get();

				auto shader = rvd.get_shader(material, 0, t->vertex_shader_module, "");
				auto& rd = rvd.start(params, tr->owner->get_matrix(), shader, material, 0, ygfx::PrimitiveTopology::TRIANGLES, vb);
#ifdef USING_VULKAN
				cb->push_constant(0, 12, &t->texture_scale[0].x);
				cb->push_constant(16, 12, &t->texture_scale[1].x);
#else
				shader->set_floats("pattern0", &t->texture_scale[0].x, 3);
				shader->set_floats("pattern1", &t->texture_scale[1].x, 3);
#endif
				rd.draw_triangles(params, vb);
			}

		const auto models = data->entity_manager->get_component_list<ModelRef>();
		for (auto mr: models)
			if (auto m = mr->model)
				for (int k=0; k<m->mesh[0]->sub.num; k++) {
					auto material = m->materials[k];
					auto vb = m->mesh[0]->sub[k].vertex_buffer;
					data->session->drawing_helper->draw_mesh(params, rvd, mr->owner->get_matrix(), vb, material, 0, "default");
				}
	}
};

class CameraPreviewRenderer {
public:
	xhui::Panel* panel;
	xhui::Window* window = nullptr;
	string id;
	DataWorld* data;
	Camera* cam;
	owned<yrenderer::XhuiRenderer> xhui_renderer;
	owned<yrenderer::RenderPath> render_path;
	owned<yrenderer::HDRResolver> hdr_resolver;
	shared<CPEmitter> cp_emitter;
	owned_array<yrenderer::Light> lights;
	int eid[3] = {-1, -1, -1};
	CameraPreviewRenderer(xhui::Panel* _panel, const string& _id, DataWorld* _data, Camera* _cam) {
		panel = _panel;
		id = _id;
		data = _data;
		cam = _cam;
		xhui_renderer = new yrenderer::XhuiRenderer(data->session->ctx);
		render_path = new yrenderer::RenderPathForward(data->session->ctx, 256);
		render_path->background_color = data->meta_data.background_color;

#if 0
		hdr_resolver = new yrenderer::HDRResolver(data->session->ctx, 300, 200, false);
		hdr_resolver->add_child(render_path.get());

		xhui_renderer->add_child(hdr_resolver.get());
#else
		xhui_renderer->add_child(render_path.get());
#endif

		cp_emitter = new CPEmitter(data);
		render_path->add_opaque_emitter({cp_emitter.get()});

		eid[0] = xhui::run_later(0.01f, [this] {
			window = panel->get_window();
			eid[1] = window->event_xp(panel->get_window()->id, xhui::event_id::JustBeforeDraw, [this] (Painter* p) {
				render_path->set_view(cam->params());

				auto data_lights = data->entity_manager->get_component_list<Light>();

				while (lights.num < data_lights.num) {
					lights.add(new yrenderer::Light);
				}
				for (const auto& [i, l]: enumerate(data_lights)) {
					*lights[i] = l->light;
					lights[i]->pos = l->owner->pos;
					lights[i]->_ang = l->owner->ang;
				}
				render_path->set_lights(weak(lights).sub_ref(0, data_lights.num));

				xhui_renderer->before_draw(p);
			});
			eid[2] = panel->event_xp(id, xhui::event_id::Draw, [this] (Painter* p) {
				xhui_renderer->draw(p);
			});
		});
	}
	~CameraPreviewRenderer() {
		xhui::cancel_runner(eid[0]);
		if (window)
			window->remove_event_handler(eid[1]);
		panel->remove_event_handler(eid[2]);
	}
};

constexpr float SENSOR_HEIGHT = 24.0f; // "35mm"
//constexpr float SENSOR_HEIGHT = 15.7f; // "APS-C"

constexpr float fov_to_35mm_focal_length(float fov) {
	return SENSOR_HEIGHT/2/tanf(fov/2);
}

constexpr float fov_from_35mm_focal_length(float f) {
	return atanf(SENSOR_HEIGHT/2/f)*2;
}

CameraPanel::CameraPanel(DataWorld* _data, int _index) : ComponentContentsPanel(_data, _index) {
	from_source(R"foodelim(
Dialog camera-panel ''
	Grid ? ''
		Grid ? ''
			Label ? 'Min distance' right disabled
			SpinButton z-min '' 'tooltip=Minimum rendering distance' range=0::0.001 expandx
			---|
			Label ? 'Max distance' right disabled
			SpinButton z-max '' 'tooltip=Maximum rendering distance' range=0::0.001
			---|
			Label ? 'Field of view' right disabled
			SpinButton fov '' 'tooltip=Vertical opening angle' range=0:180:0.1
			Label ? '°'
			---|
			Label ? 'Focal length' right disabled
			SpinButton focal-length '' 'tooltip=Equivalent focal length of a lens on a 35mm "full frame" photo camera' range=1::0.1
			Label ? 'mm'
			---|
			Label ? 'Exposure' right disabled
			SpinButton exposure '' 'tooltip=Brightness scaling factor' range=0:100:0.001
		---|
		Group g-preview 'Preview'
			DrawingArea preview '' height=220
)foodelim");
	data = _data;
	index = _index;
	preview = new CameraPreviewRenderer(this, "preview", data, data->entity(index)->get_component<Camera>());
	update_ui();

	event("z-min", [this] { on_edit(); });
	event("z-max", [this] { on_edit(); });
	event("fov", [this] {
		auto c = data->entity(index)->get_component<Camera>();
		set_float("focal-length", fov_to_35mm_focal_length(c->fov));
		on_edit();
	});
	event("focal-length", [this] {
		set_float("fov", 180/pi*fov_from_35mm_focal_length(get_float("focal-length")));
		on_edit();
	});
	event("exposure", [this] { on_edit(); });
}

CameraPanel::~CameraPanel() = default;

void CameraPanel::update_ui() {
	if (auto c = data->entity(index)->get_component<Camera>()) {
		set_float("z-min", c->min_depth);
		set_float("z-max", c->max_depth);
		set_float("fov", c->fov * 180 / pi);
		set_float("focal-length", fov_to_35mm_focal_length(c->fov));
		set_float("exposure", c->exposure);
	}
}
void CameraPanel::on_edit() {
	Camera c;
	c.min_depth = get_float("z-min");
	c.max_depth = get_float("z-max");
	c.exposure = get_float("exposure");
	c.fov = get_float("fov") * pi / 180;

	editing = true;
	auto e = data->entity(index);
	data->entity_edit_component(e, Camera::_class, data->session->plugin_manager->describe_class(Camera::_class, &c));
	editing = false;
}
