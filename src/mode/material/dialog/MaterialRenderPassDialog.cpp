/*
 * MaterialRenderPassDialog.cpp
 *
 *  Created on: 9 Nov 2023
 *      Author: michi
 */

#include "MaterialRenderPassDialog.h"


MaterialRenderPassDialog::MaterialRenderPassDialog(hui::Window *parent, const DataMaterial::RenderPassData &data) :
		hui::Dialog("material-render-pass-dialog", parent)
{
	result = data;
	load_data();
	event("transparency_mode:none", [this]{ on_transparency_mode(); });
	event("transparency_mode:function", [this]{ on_transparency_mode(); });
	event("transparency_mode:color_key", [this]{ on_transparency_mode(); });
	event("transparency_mode:factor", [this]{ on_transparency_mode(); });
	event("transparency_mode:mix", [this]{ on_transparency_mode(); });
	event("ok", [this] { on_ok(); });
	event("cancel", [this] { request_destroy(); });
	event("hui:close", [this] { request_destroy(); });
}

void MaterialRenderPassDialog::load_data() {
	if (result.mode == TransparencyMode::COLOR_KEY_SMOOTH)
		check("transparency_mode:color_key", true);
	else if (result.mode == TransparencyMode::COLOR_KEY_HARD)
		check("transparency_mode:color_key", true);
	else if (result.mode == TransparencyMode::FACTOR)
		check("transparency_mode:factor", true);
	else if (result.mode == TransparencyMode::FUNCTIONS)
		check("transparency_mode:function", true);
	else if (result.mode == TransparencyMode::MIX)
		check("transparency_mode:mix", true);
	else
		check("transparency_mode:none", true);
	enable("alpha_factor", result.mode == TransparencyMode::FACTOR);
	enable("alpha_source", result.mode == TransparencyMode::FUNCTIONS);
	enable("alpha_dest", result.mode == TransparencyMode::FUNCTIONS);
	set_float("alpha_factor", result.factor * 100.0f);
	check("alpha_z_buffer", result.z_buffer);
	set_int("alpha_source", (int)result.source);
	set_int("alpha_dest", (int)result.destination);
	if (result.culling == nix::CullMode::NONE)
		check("cull:none", true);
	else if (result.culling == nix::CullMode::FRONT)
		check("cull:front", true);
	else
		check("cull:back", true);
}

void MaterialRenderPassDialog::on_transparency_mode() {
	if (is_checked("transparency_mode:function"))
		result.mode = TransparencyMode::FUNCTIONS;
	else if (is_checked("transparency_mode:color_key"))
		result.mode = TransparencyMode::COLOR_KEY_HARD;
	else if (is_checked("transparency_mode:factor"))
		result.mode = TransparencyMode::FACTOR;
	else if (is_checked("transparency_mode:mix"))
		result.mode = TransparencyMode::MIX;
	else
		result.mode = TransparencyMode::NONE;
	load_data();
}

void MaterialRenderPassDialog::on_ok() {
	result.source = (nix::Alpha)get_int("alpha_source");
	result.destination = (nix::Alpha)get_int("alpha_dest");
	result.factor = get_float("alpha_factor") * 0.01f;
	result.z_buffer = is_checked("alpha_z_buffer");
	if (is_checked("cull:none"))
		result.culling = nix::CullMode::NONE;
	else if (is_checked("cull:front"))
		result.culling = nix::CullMode::FRONT;
	else
		result.culling = nix::CullMode::BACK;
	success = true;
	request_destroy();
}
