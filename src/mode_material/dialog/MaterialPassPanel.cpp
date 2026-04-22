//
// Created by Michael Ankele on 2025-05-02.
//

#include "MaterialPassPanel.h"
#include "MaterialPanel.h"
#include <mode_material/action/ActionMaterialEditAppearance.h>
#include <Session.h>
#include <storage/Storage.h>

string file_secure(const Path &filename);

static Array<yrenderer::TransparencyMode> transparency_modes = {
	yrenderer::TransparencyMode::NONE,
	yrenderer::TransparencyMode::FUNCTIONS,
	yrenderer::TransparencyMode::COLOR_KEY_HARD,
	yrenderer::TransparencyMode::COLOR_KEY_SMOOTH,
	yrenderer::TransparencyMode::FACTOR,
	yrenderer::TransparencyMode::MIX};
static Array<ygfx::Alpha> alphas = {
	ygfx::Alpha::ZERO,
	ygfx::Alpha::ONE,
	ygfx::Alpha::SOURCE_COLOR,
	ygfx::Alpha::SOURCE_INV_COLOR,
	ygfx::Alpha::SOURCE_ALPHA,
	ygfx::Alpha::SOURCE_INV_ALPHA,
	ygfx::Alpha::DEST_COLOR,
	ygfx::Alpha::DEST_INV_COLOR,
	ygfx::Alpha::DEST_ALPHA,
	ygfx::Alpha::DEST_INV_ALPHA};

MaterialPassPanel::MaterialPassPanel(MaterialPanel* _parent, DataMaterial* _data, int _index) : xhui::Panel("") {
	from_resource("material-pass-panel");
	parent = _parent;
	data = _data;
	index = _index;

	event("mode", [this] { apply_data(); });
	event("source", [this] { apply_data(); });
	event("destination", [this] { apply_data(); });
	event("cull:front", [this] { apply_data(); });
	event("cull:back", [this] { apply_data(); });
	event("cull:none", [this] { apply_data(); });
	event("z-write", [this] { apply_data(); });
	event("z-test", [this] { apply_data(); });
	event("shader", [this] {
		data->session->storage->file_dialog(FD_SHADERFILE, false, true).then([this] (const ComplexPath& p) {
			auto a = data->material;
			a.pass(index).shader_path = p.relative;
			data->execute(new ActionMaterialEditAppearance(a));
		});
	});
	event("clear-shader", [this] {
		auto a = data->material;
		a.pass(index).shader_path = "";
		data->execute(new ActionMaterialEditAppearance(a));
	});
	event("edit-shader", [this] {
		auto a = data->material;
		if (a.pass(index).shader_path)
			data->session->universal_edit(FD_SHADERFILE, a.pass(index).shader_path, true);
	});
	event("delete", [this] {
		auto a = data->material;
		for (int k=index; k<a.num_passes-1; k++)
			a.pass(k) = a.pass(k+1);
		a.set_num_passes(a.num_passes - 1);
		if (a.num_passes >= 1)
			data->execute(new ActionMaterialEditAppearance(a));
	});
}

void MaterialPassPanel::set_pass_no(int _index) {
	index = _index;
	update();
}

void MaterialPassPanel::update() {
	auto& p = data->material.pass(index);

	string h1 = "???", h2;

	if (p.mode == yrenderer::TransparencyMode::NONE)
		h1 = "Solid";
	if (p.mode == yrenderer::TransparencyMode::FUNCTIONS)
		h1 =  "Transparent";
	if (p.cull_mode == ygfx::CullMode::BACK)
		h2 =  "Front";
	else if (p.cull_mode == ygfx::CullMode::FRONT)
		h2 =  "Back";
	else if (p.cull_mode == ygfx::CullMode::NONE)
		h2 =  "Front & back";

	set_string("header", format("<b>%s</b>\n  <span size='small'>%s</span>", h1, h2));

	set_string("shader", file_secure(p.shader_path));
	set_int("mode", transparency_modes.find(p.mode));
	set_int("source", alphas.find(p.source));
	set_int("destination", alphas.find(p.destination));
	enable("source", p.mode == yrenderer::TransparencyMode::FUNCTIONS);
	enable("destination", p.mode == yrenderer::TransparencyMode::FUNCTIONS);
	check("cull:front", p.cull_mode == ygfx::CullMode::FRONT);
	check("cull:back", p.cull_mode == ygfx::CullMode::BACK);
	check("cull:none", p.cull_mode == ygfx::CullMode::NONE);
	check("z-write", p.z_write);
	check("z-test", p.z_test);
	enable("delete", data->material.num_passes >= 2);
	enable("edit-shader", !p.shader_path.is_empty());
}

void MaterialPassPanel::set_selected(bool selected) {
	expand("expander", selected);
}

// GUI -> data
void MaterialPassPanel::apply_data() {
	parent->apply_queue_depth ++;

	auto a = data->material;
	auto& p = a.pass(index);

	p.mode = transparency_modes[get_int("mode")];
	p.source = alphas[get_int("source")];
	p.destination = alphas[get_int("destination")];
	if (is_checked("cull:back"))
		p.cull_mode = ygfx::CullMode::BACK;
	else if (is_checked("cull:front"))
		p.cull_mode = ygfx::CullMode::FRONT;
	else if (is_checked("cull:none"))
		p.cull_mode = ygfx::CullMode::NONE;
	p.z_write = is_checked("z-write");
	p.z_test = is_checked("z-test");

	enable("source", p.mode == yrenderer::TransparencyMode::FUNCTIONS);
	enable("destination", p.mode == yrenderer::TransparencyMode::FUNCTIONS);

	data->execute(new ActionMaterialEditAppearance(a));
	this->parent->apply_queue_depth --;
}

