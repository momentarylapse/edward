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
			parent->apply_queue_depth ++;
			auto a = data->appearance;
			a.passes[index].shader.file = p.relative;
			data->execute(new ActionMaterialEditAppearance(a));
			parent->apply_queue_depth --;
			update(index);
		});
	});
	event("clear-shader", [this] {
		parent->apply_queue_depth ++;
		auto a = data->appearance;
		a.passes[index].shader.file = "";
		data->execute(new ActionMaterialEditAppearance(a));
		parent->apply_queue_depth --;
		update(index);
	});
	event("edit-shader", [this] {
		auto a = data->appearance;
		if (!a.passes[index].shader.file.is_empty())
			data->session->universal_edit(FD_SHADERFILE, a.passes[index].shader.file, true);
	});
	event("delete", [this] {
		auto a = data->appearance;
		a.passes.erase(index);
		if (a.passes.num >= 1)
			data->execute(new ActionMaterialEditAppearance(a));
	});
}

void MaterialPassPanel::update(int _index) {
	index = _index;
	auto& p = data->appearance.passes[index];

	if (p.mode == yrenderer::TransparencyMode::NONE)
		set_string("header", "Solid");
	if (p.mode == yrenderer::TransparencyMode::FUNCTIONS)
		set_string("header", "Transparent");
	if (p.culling == ygfx::CullMode::BACK)
		set_string("subheader", "Front");
	else if (p.culling == ygfx::CullMode::FRONT)
		set_string("subheader", "Back");
	else if (p.culling == ygfx::CullMode::NONE)
		set_string("subheader", "Front & back");

	set_string("shader", file_secure(p.shader.file));
	set_int("mode", transparency_modes.find(p.mode));
	set_int("source", alphas.find(p.source));
	set_int("destination", alphas.find(p.destination));
	enable("source", p.mode == yrenderer::TransparencyMode::FUNCTIONS);
	enable("destination", p.mode == yrenderer::TransparencyMode::FUNCTIONS);
	check("cull:front", p.culling == ygfx::CullMode::FRONT);
	check("cull:back", p.culling == ygfx::CullMode::BACK);
	check("cull:none", p.culling == ygfx::CullMode::NONE);
	check("z-write", p.z_write);
	check("z-test", p.z_test);
	enable("delete", data->appearance.passes.num >= 2);
	enable("edit-shader", !p.shader.file.is_empty());
}
void MaterialPassPanel::set_selected(bool selected) {
	set_visible("g-pass", selected);
}

// GUI -> data
void MaterialPassPanel::apply_data() {
	parent->apply_queue_depth ++;

	auto a = data->appearance;
	auto& p = a.passes[index];

	p.mode = transparency_modes[get_int("mode")];
	p.source = alphas[get_int("source")];
	p.destination = alphas[get_int("destination")];
	if (is_checked("cull:back"))
		p.culling = ygfx::CullMode::BACK;
	else if (is_checked("cull:front"))
		p.culling = ygfx::CullMode::FRONT;
	else if (is_checked("cull:none"))
		p.culling = ygfx::CullMode::NONE;
	p.z_write = is_checked("z-write");
	p.z_test = is_checked("z-test");

	enable("source", p.mode == yrenderer::TransparencyMode::FUNCTIONS);
	enable("destination", p.mode == yrenderer::TransparencyMode::FUNCTIONS);

	data->execute(new ActionMaterialEditAppearance(a));
	this->parent->apply_queue_depth --;
}

