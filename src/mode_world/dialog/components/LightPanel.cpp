//
// Created by michi on 9/30/25.
//

#include "LightPanel.h"
#include "../../data/DataWorld.h"
#include <y/world/components/Light.h>
#include <stuff/PluginManager.h>
#include <ecs/Entity.h>
#include <cmath>

LightPanel::LightPanel(DataWorld* _data, int _index) : Node("light-panel") {
	from_source(R"foodelim(
Dialog light-panel ''
	Grid ? ''
		Label ? 'Enabled' right disabled
		CheckBox enabled ''
		---|
		Label ? 'Type' right disabled
		ComboBox type 'Directional\\Point\\Cone\\Ambient' range=0:2:1
		---|
		Label ? 'Radius' right disabled
		SpinButton radius '' 'tooltip=Distance at which point and cone lights fall off to 10% power' range=0::0.1
		---|
		Label ? 'Theta' right disabled
		SpinButton theta '' 'tooltip=Opening angle for cone lights' range=0:180:0.1
		Label ? '°'
		---|
		Label ? 'Color' right disabled
		ColorButton color '' 'tooltip=Basic color. Will be scaled by power and distance.'
		---|
		Label ? 'Power' right disabled
		SpinButton power '' 'tooltip=Intensity scale for color (at 1 unit distance)' range=0::0.01
		---|
		Label ? 'Harshness' right disabled
		SpinButton harshness '' 'tooltip=Suppress lighting in shadowed areas' range=0:100:1
		Label ? '%'
		---|
		Label ? 'Shadows' right disabled
		CheckBox allow-shadows '' 'tooltip=Cast shadows (if allowed by mesh materials)?'
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

	data->out_changed >> create_sink([this] {
		if (!editing)
			update_ui();
	});
}

void LightPanel::update_ui() {
	auto e = data->entity(index);
	auto l = e->get_component<Light>();
	check("enabled", l->light.enabled);
	set_int("type", (int)l->light.type);
	set_float("radius", l->light.radius());
	set_float("harshness", l->light.harshness * 100);
	set_float("theta", max(l->light.theta * 180 / pi, 0.0f));
	set_color("color", l->light.col);
	set_float("power", l->light.power);
	check("allow-shadows", l->light.allow_shadow);
	enable("power", l->light.type == yrenderer::LightType::DIRECTIONAL or l->light.type == yrenderer::LightType::AMBIENT);
	enable("radius", l->light.type != yrenderer::LightType::DIRECTIONAL and l->light.type != yrenderer::LightType::AMBIENT);
	enable("theta", l->light.type == yrenderer::LightType::CONE or l->light.type == yrenderer::LightType::AMBIENT);
}

void LightPanel::on_edit() {
	yrenderer::Light l;
	l.enabled = is_checked("enabled");
	l.type = (yrenderer::LightType)get_int("type");
	enable("power", l.type == yrenderer::LightType::DIRECTIONAL or l.type == yrenderer::LightType::AMBIENT);
	enable("radius", l.type != yrenderer::LightType::DIRECTIONAL and l.type != yrenderer::LightType::AMBIENT);
	enable("theta", l.type == yrenderer::LightType::CONE or l.type == yrenderer::LightType::AMBIENT);

	l.col = get_color("color");
	float radius = get_float("radius");
	float power = get_float("power");
	if (l.type == yrenderer::LightType::DIRECTIONAL or l.type == yrenderer::LightType::AMBIENT) {
		l.power = power;
	} else {
		l.power = yrenderer::Light::_radius_to_power(radius);
		set_float("power", l.power);
	}
	if (l.type == yrenderer::LightType::CONE) {
		l.theta = get_float("theta") * pi / 180;
	} else if (l.type == yrenderer::LightType::AMBIENT) {
		l.theta = 5;
	} else {
		l.theta = -1;
	}
	l.harshness = get_float("harshness") / 100;
	l.allow_shadow = is_checked("allow-shadows");

	editing = true;
	auto e = data->entity(index);
	Light ll(yrenderer::LightType::DIRECTIONAL, White);
	ll.light = l;
	data->entity_edit_component(e, Light::_class, data->session->plugin_manager->describe_class(Light::_class, &ll));
	editing = false;
}
