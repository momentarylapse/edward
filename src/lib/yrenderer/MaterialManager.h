//
// Created by michi on 8/4/25.
//

#pragma once

#include "Material.h"
#include <lib/pattern/Observable.h>

namespace yrenderer {

class TextureManager;

class MaterialManager : public obs::Node<VirtualBase> {
public:
	explicit MaterialManager(TextureManager* texture_manager, const Path& material_dir);
	~MaterialManager() override;

	obs::xsource<Material*> out_material_edited{this, "material-edited"};

	void reset();

	void set_default(Material* m);
	Material* load(const Path& filename);
	xfer<Material> load_copy(const Path& filename);
	Path get_filename(const Material* m) const;
	string describe(const Material* m, bool with_save_state = false) const;
	void invalidate(Material* m);
	void set_save_state(Material* m);
	Material* create_internal();
	bool is_from_file(const Material* m) const;
	bool has_changes(const Material* m) const;

	Path material_dir;


	void _load_from_file(Material* material, const Path& filename);
	void _write_to_file(Material* material, const Path& filename);

private:
	TextureManager* texture_manager;
	Material* default_material;
	Material* trivial_material;
	base::map<Path, Material*> materials; // "originals" owned!
	owned_array<Material> internal_materials;
	base::set<const Material*> having_changes;
};

}
