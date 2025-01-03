/*
 * ModeMaterial.h
 *
 *  Created on: 08.03.2012
 *      Author: michi
 */

#ifndef MODEMATERIAL_H_
#define MODEMATERIAL_H_

#include "../Mode.h"
#include "../../data/material/DataMaterial.h"
#include <y/graphics-fwd.h>

class MaterialPropertiesDialog;
class ShaderGraphDialog;
class Geometry;

class ModeMaterial: public Mode<ModeMaterial, DataMaterial> {
public:
	ModeMaterial(Session *s, MultiView::MultiView *mv);
	~ModeMaterial() override;

	obs::sink in_data_changed;
	obs::source out_shader_edit_mode_changed{this, "shader-edit-mode-changed"};
	obs::source out_current_render_pass_changed{this, "current-render-pass-changed"};

	void on_start() override;
	void on_end() override;

	void on_command(const string &id) override;
	void on_data_update();

	void on_draw() override;
	void on_draw_win(MultiView::Window *win) override;

	void on_update_menu() override;


	void _new();
	void open();
	void save();
	void save_as();

	bool optimize_view() override;

	owned<VertexBuffer> vertex_buffer[MATERIAL_MAX_TEXTURES + 1];
	MaterialPropertiesDialog *appearance_dialog = nullptr;
	ShaderGraphDialog *shader_graph_dialog = nullptr;


	shared_array<Texture> textures;
	shared_array<Shader> shaders;
	void update_textures();
	void update_shader(int pass_no);

	string shape_type;
	bool shape_smooth;
	owned<Geometry> geo;

	enum class ShaderEditMode {
		NONE,
		GRAPH,
		CODE
	} shader_edit_mode;
	void set_shader_edit_mode(ShaderEditMode mode);

	void select_render_pass(int p);
	int current_render_pass = 0;


	void set_shape_type(const string &type);
	void set_shape_smooth(bool smooth);
	void update_shape();
};

#endif /* MODEMATERIAL_H_ */
