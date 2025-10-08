//
// Created by michi on 9/30/25.
//

#include "LightPanel.h"
#include "../data/DataWorld.h"
#include <y/world/Light.h>
#include <y/y/Entity.h>
#include <cmath>

LightPanel::LightPanel(DataWorld* _data, int _index) : obs::Node<xhui::Panel>("light-panel") {
	from_source(R"foodelim(
Dialog light-panel ''
	Grid ? ''
		Label ? 'Enabled' right disabled
		CheckBox enabled ''
		---|
		Label ? 'Type' right disabled
		ComboBox type 'Directional\\Point\\Cone' range=0:2:1
		---|
		Label ? 'Radius' right disabled
		SpinButton radius '' range=0::0.1
		---|
		Label ? 'Theta' right disabled
		SpinButton theta '' range=0:180:0.1
		Label ? '°'
		---|
		Label ? 'Color' right disabled
		ColorButton color ''
		---|
		Label ? 'Power' right disabled
		SpinButton power '' range=0::0.01
		---|
		Label ? 'Harshness' right disabled
		SpinButton harshness '' range=0:100:1
		Label ? '%'
		---|
		Label ? 'Shadows' right disabled
		CheckBox allow-shadows ''
)foodelim");
	data = _data;
	index = _index;
	update_ui();
	/*data->out_changed >> create_sink([this] {
		msg_write("update");
		update_ui();
	});*/

	event("enabled", [this] { on_edit(); });
	event("type", [this] { on_edit(); });
	event("radius", [this] { on_edit(); });
	event("theta", [this] { on_edit(); });
	event("color", [this] { on_edit(); });
	event("power", [this] { on_edit(); });
	event("harshness", [this] { on_edit(); });
	event("allow-shadows", [this] { on_edit(); });
}

void LightPanel::update_ui() {
	auto e = data->entity(index);
	auto l = e->get_component<Light>();
	float max_component = max(max(l->light.col.r, l->light.col.g), l->light.col.b);
	float factor = max(max_component, 1.0f); // correction for invalid colors
	check("enabled", l->light.enabled);
	set_int("type", (int)l->light.type);
	set_float("radius", l->light.radius());
	set_float("harshness", l->light.harshness * 100);
	set_float("theta", max(l->light.theta * 180 / pi, 0.0f));
	set_color("color", l->light.col * (1.0f / factor));
	set_float("power", l->light.power * factor);
	check("allow-shadows", l->light.allow_shadow);
	enable("power", l->light.type == yrenderer::LightType::DIRECTIONAL);
	enable("radius", l->light.type != yrenderer::LightType::DIRECTIONAL);
	enable("theta", l->light.type == yrenderer::LightType::CONE);
}

void LightPanel::on_edit() {
	auto e = data->entity(index);
	auto l = e->get_component<Light>();
	l->light.enabled = is_checked("enabled");
	l->light.type = (yrenderer::LightType)get_int("type");
	enable("power", l->light.type == yrenderer::LightType::DIRECTIONAL);
	enable("radius", l->light.type != yrenderer::LightType::DIRECTIONAL);
	enable("theta", l->light.type == yrenderer::LightType::CONE);

	float radius = get_float("radius");
	float power = get_float("power");
	if (l->light.type == yrenderer::LightType::DIRECTIONAL) {
		l->light.col = get_color("color") * power;
		l->light.power = power;
		l->light.theta = -1;
	} else {
		l->light.col = get_color("color");
		l->light.power = yrenderer::Light::_radius_to_power(radius);
		l->light.theta = get_float("theta") * pi / 180;
		set_float("power", l->light.power);
	}
	l->light.harshness = get_float("harshness") / 100;
	l->light.allow_shadow = is_checked("allow-shadows");
	//data->edit_light(index, l);*/
}
