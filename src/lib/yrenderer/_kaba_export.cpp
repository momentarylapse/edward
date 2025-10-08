#include "Context.h"
#include "Material.h"
#include "helper/ComputeTask.h"
#include "helper/LightMeter.h"
#include "post/HDRResolver.h"
#include "scene/Light.h"
#include "scene/CameraParams.h"
#include "scene/SceneView.h"
#include "scene/RenderViewData.h"
#include "scene/SceneRenderer.h"
#include "scene/MeshEmitter.h"
#include "scene/mesh/CubeEmitter.h"
#include "scene/path/RenderPath.h"
#include "scene/path/RenderPathForward.h"
#include "scene/path/RenderPathDeferred.h"
#include "regions/RegionRenderer.h"
#include "target/WindowRenderer.h"
#include "target/XhuiRenderer.h"
#include "../ygraphics/graphics-impl.h"
#include "../image/image.h"
#include "../kabaexport/KabaExporter.h"
#include "../os/msg.h"
#include "target/TextureRenderer.h"


#define _OFFSET(VAR, MEMBER)	(char*)&VAR.MEMBER - (char*)&VAR


//using namespace yrenderer;
using namespace ygfx;


#pragma GCC push_options
#pragma GCC optimize("no-omit-frame-pointer")
#pragma GCC optimize("no-inline")
#pragma GCC optimize("0")


void framebuffer_init(FrameBuffer *fb, const shared_array<Texture> &tex) {
#ifdef USING_VULKAN
	msg_error("not implemented: FrameBuffer.__init__() for vulkan");
	//kaba::kaba_raise_exception(new kaba::KabaException("not implemented: FrameBuffer.__init__() for vulkan"));
#else
	new(fb) FrameBuffer(tex);
#endif
}

shared<Texture> framebuffer_depthbuffer(FrameBuffer *fb) {
#ifdef USING_VULKAN
	return fb->attachments.back().get();
#else
	return fb->depth_buffer.get();
#endif
}

shared_array<Texture> framebuffer_color_attachments(FrameBuffer *fb) {
#ifdef USING_VULKAN
	return fb->attachments;//.sub_ref(0, -1));
#else
	return fb->color_attachments;
#endif
}

void buffer_update_array(Buffer *buf, const DynamicArray &data) {
	buf->update_array(data);
}

void buffer_update_chunk(Buffer *buf, const void* data, int size) {
	buf->update_part(data, 0, size);
}

void buffer_read_chunk(Buffer *buf, void* data, int size) {
#ifdef USING_VULKAN
	auto p = buf->map();
	memcpy(data, p, size);
	buf->unmap();
#else
	buf->read(data, size);
#endif
}

void buffer_read_array(Buffer *buf, DynamicArray &data) {
#ifdef USING_VULKAN
	buffer_read_chunk(buf, data.data, data.num * data.element_size);
#else
	buf->read_array(data);
#endif
}

void vertexbuffer_init(VertexBuffer *vb, const string &format) {
	new(vb) VertexBuffer(format);
}

void vertexbuffer_update_array(VertexBuffer *buf, const DynamicArray &data) {
	buf->update(data);
}

void uniformbuffer_init(UniformBuffer* buf, int size) {
	new(buf) UniformBuffer(size);
}

void storagebuffer_init(ShaderStorageBuffer* buf, int size) {
	new(buf) ShaderStorageBuffer(size);
}

void computetask_init(yrenderer::ComputeTask* task, yrenderer::Context* ctx, const string& name, const shared<Shader>& shader, const Array<int>& n) {
	int nx = 1;
	int ny = 1;
	int nz = 1;
	if (n.num >= 1)
		nx = n[0];
	if (n.num >= 2)
		ny = n[1];
	if (n.num >= 3)
		nz = n[2];
	new(task) yrenderer::ComputeTask(ctx, name, shader, nx, ny, nz);
}

void texture_init(Texture *t, int w, int h, const string &format) {
	new(t) Texture(w, h, format);
}

void texture_delete(Texture *t) {
	t->~Texture();
}

void texture_write(Texture *t, const Image &im) {
	t->write(im);
}

void texture_write_float(Texture *t, const DynamicArray& data) {
#ifdef USING_VULKAN
	int n_in = data.num * data.element_size / (int)sizeof(float);
	int n_tex = t->width * t->height * t->depth;
	if (n_in != n_tex) {
		msg_error(format("Texture.write_float(): size mismatch (input: %d - texture: %d)", n_in, n_tex));
		return;
	}
	t->writex(data.data, t->width, t->height, t->depth, "r:f32");
#else
	t->write_float(data);
#endif
}

void texture_read(Texture* t, Image& im) {
#ifdef USING_VULKAN
	t->read(im.data.data);
#else
	t->read(im);
#endif
}

void cubemap_init(CubeMap *t, int size, const string &format) {
	new(t) CubeMap(size, format);
}

void depthbuffer_init(DepthBuffer *t, int w, int h, const string &format) {
#ifdef USING_VULKAN
	new(t) DepthBuffer(w, h, format, true);
#else
	new(t) DepthBuffer(w, h, format);
#endif
}

void imagetexture_init(DepthBuffer *t, int w, int h, const string &format) {
#ifdef USING_VULKAN
	new(t) ImageTexture(w, h, 1, format);
#else
	new(t) ImageTexture(w, h, format);
#endif
}

void storagetexture_init(DepthBuffer *t, int nx, int ny, int nz, const string &format) {
#ifdef USING_VULKAN
	new(t) vulkan::StorageTexture(nx, ny, nz, format);
#else
	new(t) VolumeTexture(nx, ny, nz, format);
#endif
}

void volumetexture_init(VolumeTexture *t, int nx, int ny, int nz, const string &format) {
	new(t) VolumeTexture(nx, ny, nz, format);
}

void shader_set_float(Shader *s, const string &name, float f) {
#ifdef USING_VULKAN
	msg_error("unimplemented:  Shader.set_float()");
#else
	s->set_float(name, f);
#endif
}

void shader_set_floats(Shader *s, const string &name, float *f, int num) {
#ifdef USING_VULKAN
	msg_error("unimplemented:  Shader.set_floats()");
#else
	s->set_floats(name, f, num);
#endif
}

#pragma GCC pop_options



shared_array<Texture> hdr_resolver_get_tex_bloom(yrenderer::HDRResolver &r) {
	//Array<Texture*> hdr_resolver_get_tex_bloom(HDRResolver &r) {
	msg_write("get bloom...");
	return {r.bloom_levels[0].tex_out.get(), r.bloom_levels[1].tex_out.get(), r.bloom_levels[2].tex_out.get(), r.bloom_levels[3].tex_out.get()};
	//return {r.bloom_levels[0].tex_out.get(), r.bloom_levels[1].tex_out.get(), r.bloom_levels[2].tex_out.get(), r.bloom_levels[3].tex_out.get()};
}


void export_package_yrenderer(kaba::Exporter* ext) {
	using namespace yrenderer;

	ext->declare_class_size("Material.Pass", sizeof(Material::RenderPassData));
	ext->declare_class_element("Material.Pass.shader_path", &Material::RenderPassData::shader_path);

	ext->declare_class_size("Material.Friction", sizeof(Material::Friction));
	ext->declare_class_element("Material.Friction.sliding", &Material::Friction::sliding);
	ext->declare_class_element("Material.Friction.rolling", &Material::Friction::rolling);
	ext->declare_class_element("Material.Friction._static", &Material::Friction::_static);
	ext->declare_class_element("Material.Friction.jump", &Material::Friction::jump);

	ext->declare_class_size("Material", sizeof(Material));
	ext->link_class_func("Material.__delete__", &kaba::generic_delete<Material>);
	ext->declare_class_element("Material.textures", &Material::textures);
	ext->declare_class_element("Material.pass0", &Material::pass0);
	ext->declare_class_element("Material.albedo", &Material::albedo);
	ext->declare_class_element("Material.roughness", &Material::roughness);
	ext->declare_class_element("Material.metal", &Material::metal);
	ext->declare_class_element("Material.emission", &Material::emission);
	ext->declare_class_element("Material.cast_shadow", &Material::cast_shadow);
	ext->declare_class_element("Material.friction", &Material::friction);
	ext->declare_class_element("Material.shader_data", &Material::shader_data);

	{
		Light light;
		ext->declare_class_size("YLight", sizeof(Light));
		ext->declare_class_element("YLight.pos", &Light::pos);
		ext->declare_class_element("YLight.ang", &Light::_ang);
		ext->declare_class_element("YLight.color", &Light::col);
		ext->declare_class_element("YLight.power", &Light::power);
	//	ext->declare_class_element("YLight.radius", _OFFSET(light, light.radius));
		ext->declare_class_element("YLight.theta", &Light::theta);
		ext->declare_class_element("YLight.harshness", &Light::harshness);
		ext->declare_class_element("YLight.enabled", &Light::enabled);
		ext->declare_class_element("YLight.allow_shadow", &Light::allow_shadow);
		ext->declare_class_element("YLight.user_shadow_control", &Light::user_shadow_control);
		ext->declare_class_element("YLight.user_shadow_theta", &Light::user_shadow_theta);
		ext->declare_class_element("YLight.shadow_dist_max", &Light::shadow_dist_max);
		ext->declare_class_element("YLight.shadow_dist_min", &Light::shadow_dist_min);
		//	ext->link_class_func("Light.set_direction", &Light::set_direction);
		ext->link_class_func("YLight.init", &Light::init);
	}


	ext->declare_enum("PrimitiveTopology.TRIANGLES", PrimitiveTopology::TRIANGLES);
	ext->declare_enum("PrimitiveTopology.TRIANGLE_FAN", PrimitiveTopology::TRIANGLE_FAN);
	ext->declare_enum("PrimitiveTopology.LINES", PrimitiveTopology::LINES);
	ext->declare_enum("PrimitiveTopology.LINE_STRIP", PrimitiveTopology::LINE_STRIP);
	ext->declare_enum("PrimitiveTopology.POINTS", PrimitiveTopology::POINTS);
	ext->declare_enum("PrimitiveTopology.PATCHES", PrimitiveTopology::PATCHES);

	ext->declare_enum("CullMode.NONE", CullMode::NONE);
	ext->declare_enum("CullMode.BACK", CullMode::BACK);
	ext->declare_enum("CullMode.FRONT", CullMode::FRONT);

	ext->declare_class_size("FrameBuffer", sizeof(FrameBuffer));
	ext->declare_class_element("FrameBuffer.width", &FrameBuffer::width);
	ext->declare_class_element("FrameBuffer.height", &FrameBuffer::height);
	ext->link_class_func("FrameBuffer.__init__", &framebuffer_init);
	ext->link_class_func("FrameBuffer.depth_buffer", &framebuffer_depthbuffer);
	ext->link_class_func("FrameBuffer.color_attachments", &framebuffer_color_attachments);

	ext->link_class_func("Buffer.update", &buffer_update_array);
	ext->link_class_func("Buffer.update_chunk", &buffer_update_chunk);
	ext->link_class_func("Buffer.read", &buffer_read_array);
	ext->link_class_func("Buffer.read_chunk", &buffer_read_chunk);

	ext->declare_class_size("VertexBuffer", sizeof(VertexBuffer));
	ext->link_class_func("VertexBuffer.__init__", &vertexbuffer_init);
	ext->link_class_func("VertexBuffer.update", &vertexbuffer_update_array);

	ext->declare_class_size("UniformBuffer", sizeof(UniformBuffer));
	ext->link_class_func("UniformBuffer.__init__", &uniformbuffer_init);

	ext->declare_class_size("ShaderStorageBuffer", sizeof(ShaderStorageBuffer));
	ext->link_class_func("ShaderStorageBuffer.__init__", &storagebuffer_init);

	ext->declare_class_size("Texture", sizeof(Texture));
	ext->declare_class_element("Texture.width", &Texture::width);
	ext->declare_class_element("Texture.height", &Texture::height);
	ext->link_class_func("Texture.__init__", &texture_init);
	ext->link_class_func("Texture.__delete__", &texture_delete);
	ext->link_class_func("Texture.write", &texture_write);
	ext->link_class_func("Texture.write_float", &texture_write_float);
	ext->link_class_func("Texture.read", &texture_read);
	ext->link_class_func("Texture.set_options", &Texture::set_options);

	ext->link_class_func("CubeMap.__init__", &cubemap_init);

	ext->link_class_func("DepthBuffer.__init__", &depthbuffer_init);

	ext->link_class_func("ImageTexture.__init__", &imagetexture_init);

	ext->link_class_func("StorageTexture.__init__", &storagetexture_init);

	ext->link_class_func("VolumeTexture.__init__", &volumetexture_init);

	ext->link_class_func("Shader.set_float", &shader_set_float);
	ext->link_class_func("Shader.set_floats", &shader_set_floats);

	{
		ext->declare_class_size("RenderParams", sizeof(RenderParams));
		ext->link_class_func("RenderParams.__assign__", &kaba::generic_assign<RenderParams>);
	}

	{
		Renderer renderer(nullptr, "");
		ext->declare_class_size("Renderer", sizeof(Renderer));
	//	ext->link_class_func("Renderer.__init__", &renderer);
		ext->link_virtual("Renderer.__delete__", &kaba::generic_virtual<Renderer>::__delete__, &renderer);
		ext->link_class_func("Renderer.add_child", &Renderer::add_child);
		ext->link_virtual("Renderer.prepare", &Renderer::prepare, &renderer);
		ext->link_virtual("Renderer.draw", &Renderer::draw, &renderer);
	}
	{
		WindowRenderer wr(nullptr, nullptr, true);
		ext->declare_class_size("WindowRenderer", sizeof(WindowRenderer));
		ext->link_class_func("WindowRenderer.__init__", &kaba::generic_init_ext<WindowRenderer, yrenderer::Context*, GLFWwindow*, bool>);
		ext->link_virtual("WindowRenderer.__delete__", &kaba::generic_virtual<WindowRenderer>::__delete__, &wr);
		ext->link_class_func("WindowRenderer.start_frame", &WindowRenderer::start_frame);
		ext->link_class_func("WindowRenderer.end_frame", &WindowRenderer::end_frame);
		ext->link_class_func("WindowRenderer.create_params", &WindowRenderer::create_params);
		ext->link_virtual("WindowRenderer.prepare", &WindowRenderer::prepare, &wr);
		ext->link_virtual("WindowRenderer.draw", &WindowRenderer::draw, &wr);
	}
	{

		XhuiRenderer xr(nullptr);
		ext->declare_class_size("XhuiRenderer", sizeof(XhuiRenderer));
		ext->link_class_func("XhuiRenderer.__init__", &kaba::generic_init_ext<XhuiRenderer, yrenderer::Context*>);
		ext->link_virtual("XhuiRenderer.__delete__", &kaba::generic_virtual<XhuiRenderer>::__delete__, &xr);
		ext->link_virtual("XhuiRenderer.render", &XhuiRenderer::render, &xr);
		ext->link_class_func("XhuiRenderer.before_draw", &XhuiRenderer::before_draw);
		ext->link_class_func("XhuiRenderer.draw", &XhuiRenderer::draw);
		ext->link_class_func("XhuiRenderer.extract_params", &XhuiRenderer::extract_params);
	}
	{
		ext->declare_class_size("SceneRenderer", sizeof(SceneRenderer));
		ext->declare_class_element("SceneRenderer.background_color", &SceneRenderer::background_color);
		ext->link_class_func("SceneRenderer.__init__", &kaba::generic_init_ext<SceneRenderer, yrenderer::Context*, RenderPathType, SceneView&>);
		ext->link_class_func("SceneRenderer.set_view", &SceneRenderer::set_view);
		ext->link_class_func("SceneRenderer.add_emitter", &SceneRenderer::add_emitter);
	}
	{
		MeshEmitter emitter(nullptr, "");
		ext->declare_class_size("MeshEmitter", sizeof(MeshEmitter));
		ext->declare_class_element("MeshEmitter.ctx", &MeshEmitter::ctx);
		ext->declare_class_element("MeshEmitter._shared_ref_count", &MeshEmitter::_pointer_ref_counter);
		ext->link_class_func("MeshEmitter.__init__", &kaba::generic_init_ext<MeshEmitter, yrenderer::Context*, const string&>);
		ext->link_virtual("MeshEmitter.emit", &MeshEmitter::emit, &emitter);
	}
	{
		CubeEmitter emitter(nullptr);
		ext->declare_class_size("CubeEmitter", sizeof(CubeEmitter));
		ext->declare_class_element("CubeEmitter.material", &CubeEmitter::material);
		ext->declare_class_element("CubeEmitter.matrix", &CubeEmitter::matrix);
		ext->link_class_func("CubeEmitter.__init__", &kaba::generic_init_ext<CubeEmitter, yrenderer::Context*>);
		ext->link_class_func("CubeEmitter.set_cube", &CubeEmitter::set_cube);
		ext->link_virtual("CubeEmitter.emit", &CubeEmitter::emit, &emitter);
	}

	{
		ComputeTask ct(nullptr, "", nullptr, 0, 0, 0);
		ext->declare_class_size("RenderTask", sizeof(RenderTask));
		ext->declare_class_element("RenderTask.active", &RenderTask::active);
		//ext->link_virtual("RenderTask.prepare", &RenderTask::prepare, &ct);
		//ext->link_virtual("RenderTask.draw", &RenderTask::draw, &ct);
		ext->link_virtual("RenderTask.render", &RenderTask::render, &ct);

		ext->declare_class_size("ComputeTask", sizeof(ComputeTask));
		ext->declare_class_element("ComputeTask.nx", &ComputeTask::nx);
		ext->declare_class_element("ComputeTask.ny", &ComputeTask::ny);
		ext->declare_class_element("ComputeTask.nz", &ComputeTask::nz);
		ext->declare_class_element("ComputeTask.shader_data", &ComputeTask::bindings); // eh, close enough
		ext->link_class_func("ComputeTask.__init__", &computetask_init);
		ext->link_class_func("ComputeTask.bind_texture", &ComputeTask::bind_texture);
		ext->link_class_func("ComputeTask.bind_image", &ComputeTask::bind_image);
		ext->link_class_func("ComputeTask.bind_uniform_buffer", &ComputeTask::bind_uniform_buffer);
		ext->link_class_func("ComputeTask.bind_storage_buffer", &ComputeTask::bind_storage_buffer);
		ext->link_virtual("ComputeTask.render", &ComputeTask::render, &ct);
	}

	{
		ext->declare_class_size("TextureRenderer", sizeof(TextureRenderer));
		ext->declare_class_element("TextureRenderer.clear_z", &TextureRenderer::clear_z);
		ext->declare_class_element("TextureRenderer.clear_colors", &TextureRenderer::clear_colors);
		ext->link_class_func("TextureRenderer.__init__", &kaba::generic_init_ext<TextureRenderer, yrenderer::Context*, const string&, const shared_array<Texture>&, const Array<string>&>);
		ext->link_class_func("TextureRenderer.set_area", &TextureRenderer::set_area);
	}

	{
		ext->declare_class_size("RegionsRenderer", sizeof(yrenderer::RegionRenderer));
		ext->declare_class_element("RegionRenderer.regions", &yrenderer::RegionRenderer::regions);
		ext->link_class_func("RegionRenderer.add_region", &yrenderer::RegionRenderer::add_region);

		ext->declare_class_size("RegionRenderer.Region", sizeof(yrenderer::RegionRenderer::Region));
		ext->declare_class_element("RegionRenderer.Region.dest", &yrenderer::RegionRenderer::Region::dest);
		ext->declare_class_element("RegionRenderer.Region.z", &yrenderer::RegionRenderer::Region::z);
		ext->declare_class_element("RegionRenderer.Region.renderer", &yrenderer::RegionRenderer::Region::renderer);
	}

	{
		ext->declare_class_size("SceneView", sizeof(SceneView));
		ext->declare_class_element("SceneView.main_camera_params", &SceneView::main_camera_params);
		ext->declare_class_element("SceneView.surfel_buffer", &SceneView::surfel_buffer);
		ext->declare_class_element("SceneView.num_surfels", &SceneView::num_surfels);
		ext->declare_class_element("SceneView.probe_cells", &SceneView::probe_cells);
		ext->declare_class_element("SceneView.probe_min", &SceneView::probe_min);
		ext->declare_class_element("SceneView.probe_max", &SceneView::probe_max);
		ext->declare_class_element("SceneView.ray_tracing_data", &SceneView::ray_tracing_data);
		ext->link_class_func("SceneView.__init__", &kaba::generic_init<SceneView>);
		ext->link_class_func("SceneView.choose_lights", &SceneView::choose_lights);
		ext->link_class_func("SceneView.choose_shadows", &SceneView::choose_shadows);
	}

	{
		ext->declare_class_size("RenderViewData", sizeof(RenderViewData));
		//ext->declare_class_element("RenderViewData.x", &RenderViewData::c);
		ext->link_class_func("RenderViewData.set_view", &RenderViewData::set_view);
		ext->link_class_func("RenderViewData.__init__", &kaba::generic_init_ext<RenderViewData, yrenderer::Context*>);
	}

	{
		RenderPath rp(nullptr, "");
		ext->declare_class_size("RenderPath", sizeof(RenderPath));
		ext->declare_class_element("RenderPath.view", &RenderPath::view);
		ext->declare_class_element("RenderPath.background_color", &RenderPath::background_color);
		ext->declare_class_element("RenderPath.ambient_occlusion_radius", &RenderPath::ambient_occlusion_radius);
		ext->link_class_func("RenderPath.set_lights", &RenderPath::set_lights);
		ext->link_class_func("RenderPath.set_view", &RenderPath::set_view);
		ext->link_virtual("RenderPath.add_background_emitter", &RenderPath::add_background_emitter, &rp);
		ext->link_virtual("RenderPath.add_opaque_emitter", &RenderPath::add_opaque_emitter, &rp);
		ext->link_virtual("RenderPath.add_transparent_emitter", &RenderPath::add_transparent_emitter, &rp);
		ext->link("RenderPath.light_sources_module", &RenderPath::light_sources_module);
		ext->link("RenderPath.lighting_method", &RenderPath::lighting_method);
		ext->link("RenderPath.shadow_method", &RenderPath::shadow_method);
	}
	{
		RenderPathForward fw(nullptr, 1024);
		ext->declare_class_size("RenderPathForward", sizeof(RenderPathForward));
		ext->link_class_func("RenderPathForward.__init__", &kaba::generic_init_ext<RenderPathForward, yrenderer::Context*, int>);
		ext->link_virtual("RenderPathForward.add_background_emitter", &RenderPathForward::add_background_emitter, &fw);
		ext->link_virtual("RenderPathForward.add_opaque_emitter", &RenderPathForward::add_opaque_emitter, &fw);
		ext->link_virtual("RenderPathForward.add_transparent_emitter", &RenderPathForward::add_transparent_emitter, &fw);
	}
	{
		ext->declare_class_size("RenderPathDeferred", sizeof(RenderPathDeferred));
		ext->link_class_func("RenderPathDeferred.__init__", &kaba::generic_init_ext<RenderPathDeferred, yrenderer::Context*, int, int, int>);
	}

	{
		ext->declare_class_size("HDRResolver.BloomLevel", sizeof(HDRResolver::BloomLevel));
		ext->declare_class_element("HDRResolver.BloomLevel.tex_out", &HDRResolver::BloomLevel::tex_out);

		ext->declare_class_size("HDRResolver", sizeof(HDRResolver));
		ext->declare_class_element("HDRResolver.exposure", &HDRResolver::exposure);
		ext->declare_class_element("HDRResolver.bloom_factor", &HDRResolver::bloom_factor);
		ext->declare_class_element("HDRResolver.texture", &HDRResolver::texture);
		ext->declare_class_element("HDRResolver.depth_buffer", &HDRResolver::depth_buffer);
		ext->declare_class_element("HDRResolver.bloom_levels", &HDRResolver::bloom_levels);
		ext->link_class_func("HDRResolver.__init__", &kaba::generic_init_ext<HDRResolver, yrenderer::Context*, int, int>);
		//ext->link_class_func("HDRResolver.tex_bloom", &hdr_resolver_get_tex_bloom);
	}

	ext->declare_class_size("LightMeter", sizeof(LightMeter));
	ext->declare_class_element("LightMeter.histogram", &LightMeter::histogram);
	ext->declare_class_element("LightMeter.brightness", &LightMeter::brightness);


	ext->declare_class_size("Context", sizeof(yrenderer::Context));
	ext->declare_class_element("Context.ctx", &yrenderer::Context::context);
	ext->declare_class_element("Context.tex_white", &yrenderer::Context::tex_white);
	ext->link_class_func("Context.create_managers", &yrenderer::Context::create_managers);
	ext->link_class_func("Context.load_material", &yrenderer::Context::load_material);
	ext->link_class_func("Context.load_texture", &yrenderer::Context::load_texture);
	ext->link_class_func("Context.load_shader", &yrenderer::Context::load_shader);
	ext->link_class_func("Context.create_shader", &yrenderer::Context::create_shader);
	ext->link_class_func("Context.load_shader_module", &yrenderer::Context::load_shader_module);
	ext->link_class_func("Context.load_surface_shader", &yrenderer::Context::load_surface_shader);


	ext->link_func("api_init_glfw", &api_init_glfw);
	ext->link_func("api_init_xhui", &api_init_xhui);
}


