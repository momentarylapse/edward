//
// Created by michi on 7/19/26.
//

#include "PluginManager.h"
#include "plugins/PluginManager.h"

#include <lib/kaba/kaba.h>
#include <lib/any/conversion.h>
#include <lib/image/color.h>
#include <lib/math/mat3.h>
#include <lib/math/vec3.h>

#include "lib/yrenderer/Material.h"

namespace plugin {

std::function<Any(const void*, const kaba::Class*)> f_whatever_to_any_special;
std::function<bool(void*, const kaba::Class*, const Any&)> f_whatever_from_any_special;

// deprecated
Array<InstanceDataVariable> parse_variables_old(const string &var) {
	Array<InstanceDataVariable> r;
	const auto xx = var.explode(",");
	for (const auto& x: xx) {
		auto y = x.explode(":");
		auto name = y[0].trim().lower().replace("_", "");
		r.add({name, Any::parse(y[1])});
	}
	return r;
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
	if (f_whatever_to_any_special) {
		const auto r = f_whatever_to_any_special(p, c);
		if (!r.is_empty())
			return r;
	}
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
	if (f_whatever_from_any_special)
		if (f_whatever_from_any_special(p, type, value))
			return;
	if (type->is_list() and value.is_list()) {
		if (!type->param[0]->can_memcpy())
			return;
		auto arr = (DynamicArray*)p;
		arr->simple_resize(value.length());
		for (int i=0; i<value.length(); i++)
			whatever_from_any(arr->simple_element(i), type->param[0], value[i]);
	}
}

void assign_variables(void* p, const kaba::Class* c, const Array<InstanceDataVariable>& variables) {
	for (const auto& v: variables)
		for (const auto& e: c->elements)
			if (v.name == e.name) {
				//msg_write("  " + e.type->long_name() + " " + e.name + " = " + v.value);
				whatever_from_any((char*)p + e.offset, e.type, v.value);
			}
}
} // plugin