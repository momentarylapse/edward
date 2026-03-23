#include "MaterialSelector.h"
#include "MaterialParameterPanel.h"
#include "MaterialSelectionDialog.h"
#include <Session.h>
#include <view/MaterialPreviewManager.h>
#include <lib/yrenderer/MaterialManager.h>
#include <data/Data.h>
#include <y/helper/ResourceManager.h>
#include <storage/format/Format.h>


MaterialSelector::MaterialSelector(Data* _data) : Node("material-selector") {
	from_source(R"foodelim(
Dialog material-selector ''
	Grid ? ''
		Grid ? ''
			Image preview '' width=48 height=48 noexpandx
			Button material '' 'tooltip=Select a material'
			ToggleButton edit-internal 'E' 'tooltip=Edit material' primary noexpandx
		---|
		Expander expander ''
			Grid editor-grid ''
				.
				---|
				Grid ? ''
					Label ? '' expandx
					Button save 'Save' 'tooltip=Save material'
					Button edit 'Edit' primary 'tooltip=Fully edit material'
)foodelim");
	data = _data;
	session = data->session;
	edit_panel = new MaterialParameterPanel(data, nullptr);
	embed("editor-grid", 0, 0, edit_panel);

	event("material", [this] {
		MaterialSelectionDialog::ask(session, "Select material", {}, false, false).then([this] (yrenderer::Material* m) {
			set_material(m);
			out_selected(material);
		});
	});
	event("edit-internal", [this] {
		expand("expander", is_checked("edit-internal"));
	});
	event("edit", [this] {
		session->universal_edit(FD_MATERIAL, session->resource_manager->material_manager->get_filename(material).with(".material"), true);
	});
	event("save", [this] {
		session->error("TODO");
	});
}

void MaterialSelector::set_material(yrenderer::Material* m) {
	material = m;
	set_string("preview", session->material_preview_manager->get(material));
	set_string("material", str(session->ctx->material_manager->get_filename(material)));
	edit_panel->set_material(material);
}
