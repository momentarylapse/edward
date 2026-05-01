/*
 * PluginManager.cpp
 *
 *  Created on: 02.01.2020
 *      Author: michi
 */

#include "PluginManager.h"
#include <lib/kaba/kaba.h>
#include <lib/kaba/lib/extern.h>
#include "../audio/SoundSource.h"
#include "../audio/AudioBuffer.h"
#include "../audio/AudioStream.h"
#include "../audio/Listener.h"
#include "../fx/Particle.h"
#include "../fx/Beam.h"
#include "../fx/ParticleEmitter.h"
#include "../fx/ParticleManager.h"
#include "../gui/gui.h"
#include "../gui/Node.h"
#include "../gui/Picture.h"
#include "../gui/Text.h"
#include "../gui/Canvas.h"
#include "../helper/DeletionQueue.h"
#include "../helper/ResourceManager.h"
#include <lib/yrenderer/ShaderManager.h>
#include <lib/yrenderer/_kaba_export.h>
#include <lib/profiler/_kaba_export.h>
#include "../helper/Scheduler.h"
#include "lib/any/conversion.h"
#include "lib/yrenderer/MaterialManager.h"
#if __has_include("../input/InputManager.h")
#include "../input/InputManager.h"
#include "../input/Gamepad.h"
#include "../input/Keyboard.h"
#include "../input/Mouse.h"
#include "../input/VR.h"
#define HAS_INPUT
#endif
#include "../net/NetworkManager.h"
#include <lib/yrenderer/Context.h>
#include "../renderer/helper/RendererFactory.h"
#include <lib/yrenderer/helper/CubeMapSource.h>
#include <lib/yrenderer/scene/SceneRenderer.h>
#include "../renderer/FullCameraRenderer.h"
#include <lib/yrenderer/scene/path/RenderPathForward.h>
#include <lib/yrenderer/scene/path/RenderPathDeferred.h>
#include "../renderer/gui/GuiRenderer.h"
#include <lib/yrenderer/post/PostProcessor.h>
#include <renderer/helper/Raytracing.h>
#include <lib/yrenderer/scene/SceneView.h>
#include <EngineData.h>
#include <ecs/Component.h>
#include <ecs/ComponentManager.h>
#include <ecs/System.h>
#include <ecs/SystemManager.h>
#include "../world/components/Camera.h"
#include "../world/components/Link.h"
#include "../world/Model.h"
#include "../world/ModelManager.h"
#include "../world/Terrain.h"
#include "../world/World.h"
#include "../world/systems/Physics.h"
#include "../world/systems/AnimationManager.h"
#include "../world/components/Light.h"
#include "../world/components/RigidBody.h"
#include "../world/components/Collider.h"
#include "../world/components/Animator.h"
#include "../world/components/Skeleton.h"
#include "../world/components/UserMesh.h"
#include "../world/components/MultiInstance.h"
#include "../world/components/CubeMapSource.h"
#include <lib/ygraphics/graphics-impl.h>
#include <lib/ygraphics/Context.h>
#include <lib/kaba/dynamic/exception.h>
#include <lib/os/msg.h>
#include <lib/image/image.h>
#include <lib/any/conversion.h>
#include <ecs/EntityManager.h>
#include <ecs/BaseClass.h>


namespace PluginManager {

//using namespace yrenderer;
using namespace ygfx;

ResourceManager* default_resource_manager = nullptr;

void init() {
	kaba::default_context->register_package_init("yengine", engine.script_dir | "yengine", &export_kaba_package_yengine);
	import_kaba();
}

template<class C>
void import_component_class(shared<kaba::Module> m, const string& name, const string& base_class = "ecs.Component") {
	for (auto c: m->classes()) {
		if (c->name == name)
			C::_class = c;
	}
	if (!C::_class)
		throw Exception(format("yengine.kaba: %s missing", name));
	if (!C::_class->is_derived_from_s(base_class))
		throw Exception(format("yengine.kaba: %s not derived from %s", name, base_class));
}

void import_kaba() {
	auto m_model = kaba::default_context->load_module("yengine/model.kaba");
	import_component_class<Animator>(m_model, "Animator");
	import_component_class<Skeleton>(m_model, "Skeleton");
	import_component_class<ModelRef>(m_model, "ModelRef");
	//import_component_class<Model>(m_model, "Model");

	auto m_world = kaba::default_context->load_module("yengine/world.kaba");
	import_component_class<RigidBody>(m_world, "RigidBody");
	import_component_class<Collider>(m_world, "Collider");
	import_component_class<MeshCollider>(m_world, "MeshCollider");
	import_component_class<SphereCollider>(m_world, "SphereCollider");
	import_component_class<BoxCollider>(m_world, "BoxCollider");
	import_component_class<TerrainCollider>(m_world, "TerrainCollider");
	import_component_class<MultiInstance>(m_world, "MultiInstance");
//	import_component_class<Terrain>(m_world, "Terrain");
	import_component_class<Light>(m_world, "Light");
	import_component_class<Camera>(m_world, "Camera");
	import_component_class<::CubeMapSource>(m_world, "CubeMapSource");
	import_component_class<NameTag>(m_world, "NameTag");
	import_component_class<TerrainRef>(m_world, "TerrainRef");
	import_component_class<TemplateRef>(m_world, "TemplateRef");
	import_component_class<EgoMarker>(m_world, "EgoMarker");
	import_component_class<Link>(m_world, "Link");
	import_component_class<Physics>(m_world, "Physics", "ecs.System"); // well, not a Component... but ok
	import_component_class<AnimationManager>(m_world, "AnimationManager", "ecs.System");
	import_component_class<ParticleManager>(m_world, "ParticleManager", "ecs.System");

	auto m_fx = kaba::default_context->load_module("yengine/fx.kaba");
	import_component_class<ParticleGroup>(m_fx, "ParticleGroup");
	import_component_class<ParticleEmitter>(m_fx, "ParticleEmitter");
	import_component_class<LegacyParticle>(m_fx, "LegacyParticle");
	import_component_class<LegacyBeam>(m_fx, "LegacyBeam");

	auto m_audio = kaba::default_context->load_module("yengine/audio.kaba");
	import_component_class<audio::SoundSource>(m_audio, "SoundSource");
	import_component_class<audio::Listener>(m_audio, "Listener");
	import_component_class<audio::Manager>(m_audio, "AudioManager", "ecs.System");

	auto m_net = kaba::default_context->load_module("yengine/networking.kaba");
	import_component_class<NetworkManager>(m_net, "NetworkManager", "ecs.System");

	auto m_y = kaba::default_context->load_module("yengine/yengine.kaba");
	import_component_class<UserMesh>(m_y, "UserMesh");

	//msg_write(MeshCollider::_class->name);
	//msg_write(MeshCollider::_class->parent->name);
	//msg_write(MeshCollider::_class->parent->parent->name);
}

Array<ecs::InstanceDataVariable> parse_variables(const string &var) {
	Array<ecs::InstanceDataVariable> r;
	auto xx = var.explode(",");
	for (auto &x: xx) {
		auto y = x.explode(":");
		auto name = y[0].trim().lower().replace("_", "");
			r.add({name, Any::parse(y[1])});
	}
	return r;
}

vec3 s2v(const string &s) {
	auto x = s.explode(" ");
	if (x.num < 3)
		return vec3::ZERO;
	return vec3(x[0]._float(), x[1]._float(), x[2]._float());
}

color s2c(const string &s) {
	auto x = s.explode(" ");
	return color(x[3]._float(), x[0]._float(), x[1]._float(), x[2]._float());
}

mat3 s2mat3(const string &s) {
	const auto a = Any::parse(s);
	mat3 m = mat3::ZERO;
	if (a.is_list())
		for (int i=0; i<min(9, a.length()); i++)
			m.e[i] = a[i].to_f32();
	return m;
}

Any whatever_to_any(const void* p, const kaba::Class* c) {
	if (!p)
		return {};
	if (c == kaba::common_types.string)
		return *(const string*)p;
	if (c == kaba::common_types.path)
		return str(*(const Path*)p);
	if (c == kaba::common_types.f32)
		return *(const float*)p;
	if (c == kaba::common_types.i32 or c->is_enum())
		return *(const int*)p;
	if (c == kaba::common_types._bool)
		return *(const bool*)p;
	if (c == kaba::common_types.vec3)
		return vec3_to_any(*(const vec3*)p);
	if (c == kaba::common_types.color)
		return color_to_any(*(const color*)p);
	if (c == kaba::common_types.mat3)
		return mat3_to_any(*(const mat3*)p);
	if ((c->name == "Material*" or c->name == "Material&") and default_resource_manager)
		return str(default_resource_manager->filename(*(const yrenderer::Material**)p));
	if (c->name == "Terrain*" and default_resource_manager)
		return str(default_resource_manager->filename(*(const Terrain**)p));
	if (c->name == "Model*" and default_resource_manager)
		return str(default_resource_manager->filename(*(const Model**)p));
	if ((c->name == "Template*" or c->name == "Template&") and default_resource_manager)
		return str(default_resource_manager->filename(*(const Template**)p));
	if (c->is_list()) {
		Any r = Any::EmptyList;
		auto arr = (DynamicArray*)p;
		for (int i=0; i<arr->num; i++)
			r.add(whatever_to_any(arr->simple_element(i), c->param[0]));
		return r;
	}
	return {};
}

void whatever_from_any(void* p, const kaba::Class* type, const Any& value) {
	if (type == kaba::common_types.string)
		*(string*)p = str(value);
	if (type == kaba::common_types.path)
		*(Path*)p = str(value);
	if (type == kaba::common_types.f32)
		*(float*)p = value.to_f32();
	if (type == kaba::common_types.i32 or type->is_enum())
		*(int*)p = value.to_i32();
	if (type == kaba::common_types._bool)
		*(bool*)p = value.to_bool();
	if (type == kaba::common_types.vec3)
		*(vec3*)p = any_to_vec3(value);
	if (type == kaba::common_types.color)
		*(color*)p = any_to_color(value);
	if (type == kaba::common_types.mat3)
		*(mat3*)p = any_to_mat3(value);
	if ((type->name == "Material*" or type->name == "Material&") and default_resource_manager)
		*(yrenderer::Material**)p = default_resource_manager->load_material(str(value));
	if (type->name == "Terrain*" and default_resource_manager)
		*(Terrain**)p = default_resource_manager->load_terrain(str(value));
	if (type->name == "Model*" and default_resource_manager)
		*(Model**)p = default_resource_manager->load_model(str(value));
	if ((type->name == "Template*" or type->name == "Template&") and default_resource_manager)
		*(Template**)p = default_resource_manager->load_template(str(value));
	if (type->is_list() and value.is_list()) {
		if (!type->param[0]->can_memcpy())
			return;
		auto arr = (DynamicArray*)p;
		arr->simple_resize(value.length());
		for (int i=0; i<value.length(); i++)
			whatever_from_any(arr->simple_element(i), type->param[0], value[i]);
	}
}

void assign_variables(void* p, const kaba::Class* c, const Array<ecs::InstanceDataVariable>& variables) {
	for (const auto& v: variables)
		for (const auto& e: c->elements)
			if (v.name == e.name) {
				//msg_write("  " + e.type->long_name() + " " + e.name + " = " + v.value);
				whatever_from_any((char*)p + e.offset, e.type, v.value);
			}
}

const kaba::Class *find_class_derived(const Path &filename, const string &base_class) {
	//msg_write(format("INSTANCE  %s:   %s", filename, base_class));
	try {
		auto s = kaba::default_context->load_module(filename);
		for (auto c: s->classes()) {
			if (c->is_derived_from_s(base_class)) {
				return c;
			}
		}
	} catch (kaba::Exception &e) {
		msg_error(e.message());
		throw Exception(e.message());
	}
	throw Exception(format("script does not contain a class derived from '%s'", base_class));
	return nullptr;
}

const kaba::Class *find_class(const Path &filename, const string &name) {
	//msg_write(format("INSTANCE  %s:   %s", filename, base_class));
	if (filename.is_empty()) {
		if (name == "Camera")
			return Camera::_class;
		if (name == "Light")
			return Light::_class;
		if (name == "Link")
			return Link::_class;
		if (name == "ModelRef")
			return ModelRef::_class;
		if (name == "TerrainRef")
			return TerrainRef::_class;
		if (name == "TemplateRef")
			return TemplateRef::_class;
		if (name == "RigidBody")
			return RigidBody::_class;
		if (name == "Skeleton")
			return Skeleton::_class;
		if (name == "Animator")
			return Animator::_class;
		if (name == "MeshCollider")
			return MeshCollider::_class;
		if (name == "TerrainCollider")
			return TerrainCollider::_class;
		if (name == "BoxCollider")
			return BoxCollider::_class;
		if (name == "SphereCollider")
			return SphereCollider::_class;
		if (name == "EgoMarker")
			return EgoMarker::_class;
	}
	try {
		auto s = kaba::default_context->load_module(filename);
		for (auto c: s->classes()) {
			if (c->name == name) {
				return c;
			}
		}
	} catch (kaba::Exception &e) {
		msg_error(e.message());
		throw;
	}
	throw Exception(format("script does not contain a class named '%s'", name));
	return nullptr;
}

void *create_instance(const kaba::Class *c, const string &variables) {
	return create_instance(c, parse_variables(variables));
}

// yes, we could skip the special cases... but then we need to export virtual functions properly...
void* create_instance(const kaba::Class *c, const Array<ecs::InstanceDataVariable> &variables) {
	//msg_write(format("INSTANCE  %s:   %s", filename, base_class));
	msg_write(format("creating instance  %s", c->long_name()));
	if (c == RigidBody::_class)
		return new RigidBody;
	if (c == ModelRef::_class)
		return new ModelRef;
	if (c == TerrainRef::_class)
		return new TerrainRef;
	if (c == TemplateRef::_class)
		return new TemplateRef;
	if (c == MeshCollider::_class)
		return new MeshCollider;
	if (c == TerrainCollider::_class)
		return new TerrainCollider;
	if (c == SphereCollider::_class)
		return new SphereCollider;
	if (c == BoxCollider::_class)
		return new BoxCollider;
//	if (c == Terrain::_class)
//		return new Terrain;
	if (c == Animator::_class)
		return new Animator;
	if (c == Skeleton::_class)
		return new Skeleton;
	if (c == Light::_class)
		return new Light(yrenderer::LightType::POINT, White);
	if (c == Link::_class)
		return new Link;
	if (c == Camera::_class)
		return new Camera;
	if (c == audio::SoundSource::_class)
		return new audio::SoundSource;
	if (c == LegacyParticle::_class)
		return new LegacyParticle;
	if (c == LegacyBeam::_class)
		return new LegacyBeam;
	if (c == CubeMapSource::_class)
		return new CubeMapSource;
	if (c == NameTag::_class)
		return new NameTag;
	void *p = c->create_instance();
	assign_variables(p, c, variables);
	return p;
}

void* create_instance_auto(const string& extended_type_name) {
	auto x = extended_type_name.explode(".");
	string& type = x.back();

	for (auto m: weak(kaba::default_context->public_modules)) {
		for (auto c: m->classes())
			if (c->name == type) {
				msg_error("create: " + c->long_name());
				return create_instance(c, "");
			}
	}
	return nullptr;
}




string callable_name(const void *c) {
	auto t = kaba::default_context->get_dynamic_type((const VirtualBase*)c);
	if (!t)
		return "callable:" + p2s(c);
	static const bool EXTRACT_FUNCTION_NAME = false;
	if (t->is_callable_bind()) {
		if (EXTRACT_FUNCTION_NAME) {
			for (auto &e: t->elements)
				if (e.name == "_fp")
					return "BIND:" + callable_name(*(const char**)((const char*)c + e.offset));
			return "kaba:bind:" + t->name_space->name;
		}
		return t->name_space->owner->module->filename.basename();
	}
	if (t->is_callable_fp()) {
		if (EXTRACT_FUNCTION_NAME) {
			for (auto &e: t->elements)
				if (e.name == "_fp") {
					auto func = *(const kaba::Function**)((const char*)c + e.offset);
					return "FUNC:" + func->long_name();
				}
			return "func:" + t->long_name();
		}
		return t->name_space->owner->module->filename.basename();//relative_to(engine.script_dir).str();
	}
	return "callable:" + p2s(c);
}


}


