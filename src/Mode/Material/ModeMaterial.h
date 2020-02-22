/*
 * ModeMaterial.h
 *
 *  Created on: 08.03.2012
 *      Author: michi
 */

#ifndef MODEMATERIAL_H_
#define MODEMATERIAL_H_

#include "../Mode.h"
#include "../../Data/Material/DataMaterial.h"

class MaterialPropertiesDialog;
class ShaderGraphDialog;
class Geometry;
namespace nix {
	class VertexBuffer;
};

class ModeMaterial: public Mode<DataMaterial> {
public:
	ModeMaterial();
	virtual ~ModeMaterial();

	void on_start() override;
	void on_end() override;

	void on_command(const string &id) override;
	void on_data_update();

	void on_draw() override;
	void on_draw_win(MultiView::Window *win) override;

	void on_update_menu() override;


	void _new() override;
	bool open() override;
	bool save() override;
	bool save_as() override;

	bool optimize_view() override;

	nix::VertexBuffer *MaterialVB[MATERIAL_MAX_TEXTURES + 1];
	MaterialPropertiesDialog *appearance_dialog;
	ShaderGraphDialog *shader_graph_dialog;

	string shape_type;
	bool shape_smooth;
	Geometry *geo;

	enum class ShaderEditMode {
		NONE,
		GRAPH,
		CODE
	} shader_edit_mode;
	void set_shader_edit_mode(ShaderEditMode mode);


	void set_shape_type(const string &type);
	void set_shape_smooth(bool smooth);
	void update_shape();
};

extern ModeMaterial* mode_material;

#endif /* MODEMATERIAL_H_ */
