//
// Created by Michael Ankele on 2025-05-02.
//

#include "MaterialPassPanel.h"
#include "MaterialPanel.h"
#include <mode_material/action/ActionMaterialEditAppearance.h>

string file_secure(const Path &filename);

static Array<TransparencyMode> transparency_modes = {TransparencyMode::NONE, TransparencyMode::FUNCTIONS, TransparencyMode::COLOR_KEY_HARD, TransparencyMode::COLOR_KEY_SMOOTH, TransparencyMode::FACTOR, TransparencyMode::MIX};
static Array<Alpha> alphas = {Alpha::ZERO, Alpha::ONE, Alpha::SOURCE_COLOR, Alpha::SOURCE_INV_COLOR, Alpha::SOURCE_ALPHA, Alpha::SOURCE_INV_ALPHA, Alpha::DEST_COLOR, Alpha::DEST_INV_COLOR, Alpha::DEST_ALPHA, Alpha::DEST_INV_ALPHA};

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
}

void MaterialPassPanel::update(int _index) {
	index = _index;
	auto& p = data->appearance.passes[index];

	if (p.mode == TransparencyMode::NONE)
		set_string("header", "Solid");
	if (p.mode == TransparencyMode::FUNCTIONS)
		set_string("header", "Transparent");
	if (p.culling == CullMode::BACK)
		set_string("subheader", "Front");
	else if (p.culling == CullMode::FRONT)
		set_string("subheader", "Back");
	else if (p.culling == CullMode::NONE)
		set_string("subheader", "Front & back");

	set_string("shader", file_secure(p.shader.file));
	set_int("mode", transparency_modes.find(p.mode));
	set_int("source", alphas.find(p.source));
	set_int("destination", alphas.find(p.destination));
	enable("source", p.mode == TransparencyMode::FUNCTIONS);
	enable("destination", p.mode == TransparencyMode::FUNCTIONS);
	check("cull:front", p.culling == CullMode::FRONT);
	check("cull:back", p.culling == CullMode::BACK);
	check("cull:none", p.culling == CullMode::NONE);
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
		p.culling = CullMode::BACK;
	else if (is_checked("cull:front"))
		p.culling = CullMode::FRONT;
	else if (is_checked("cull:none"))
		p.culling = CullMode::NONE;

	enable("source", p.mode == TransparencyMode::FUNCTIONS);
	enable("destination", p.mode == TransparencyMode::FUNCTIONS);

	data->execute(new ActionMaterialEditAppearance(a));
	this->parent->apply_queue_depth --;
}

