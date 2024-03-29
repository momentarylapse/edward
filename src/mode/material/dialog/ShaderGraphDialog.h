/*
 * ShaderGraphDialog.h
 *
 *  Created on: 21.02.2020
 *      Author: michi
 */

#ifndef SRC_MODE_MATERIAL_DIALOG_SHADERGRAPHDIALOG_H_
#define SRC_MODE_MATERIAL_DIALOG_SHADERGRAPHDIALOG_H_

#include "../../../lib/hui/hui.h"
#include "../../../lib/math/rect.h"
#include "../../../lib/math/vec2.h"
#include <data/material/DataMaterial.h>

class DataMaterial;
class ShaderGraph;
class ShaderNode;

class ShaderGraphDialog : public obs::Node<hui::Panel> {
public:
	ShaderGraphDialog(DataMaterial *data);
	virtual ~ShaderGraphDialog();

	obs::sink in_data_changed;

	void draw_node(Painter *p, ShaderNode *n);
	void draw_cable(Painter *p, ShaderNode *source, int source_port, ShaderNode *dest, int dest_port);

	void on_draw(Painter *p);

	void on_left_button_down();
	void on_left_button_up();
	void on_right_button_down();
	void on_mouse_move();
	void on_mouse_wheel();
	void on_key_down();

	void on_update();
	void on_reset();
	void on_data_changed();

	void changed();

	void request_optimal_view();
	bool _optimal_view_requested;
	void _optimize_view(const rect &area);

	struct HoverData {
		enum class Type {
			NONE,
			VIEW,
			NODE,
			PORT_IN,
			PORT_OUT,
			PARAMETER
		};
		Type type;
		ShaderNode *node;
		int port;
		int param;
		HoverData();
	};
	HoverData hover;
	HoverData selection;
	HoverData get_hover();

	DataMaterial *data;
	ShaderGraph *graph;

	int current_render_pass = 0;
	void set_current_render_pass(int p);
	DataMaterial::RenderPassData &pass();

	void update_mouse();
	vec2 m;
	vec2 move_d;
	float view_scale;
	vec2 view_offset;
	vec2 proj(const vec2 &p);


	hui::Menu *popup;
};

#endif /* SRC_MODE_MATERIAL_DIALOG_SHADERGRAPHDIALOG_H_ */
