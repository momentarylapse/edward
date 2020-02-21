/*
 * ShaderGraphDialog.h
 *
 *  Created on: 21.02.2020
 *      Author: michi
 */

#ifndef SRC_MODE_MATERIAL_DIALOG_SHADERGRAPHDIALOG_H_
#define SRC_MODE_MATERIAL_DIALOG_SHADERGRAPHDIALOG_H_

#include "../../../lib/hui/hui.h"

class DataMaterial;
class ShaderGraph;
class ShaderNode;

class ShaderGraphDialog : public hui::Dialog {
public:
	ShaderGraphDialog(hui::Window *parent, DataMaterial *data);
	virtual ~ShaderGraphDialog();

	void draw_node(Painter *p, ShaderNode *n);

	void on_draw(Painter *p) override;

	void on_left_button_down();
	void on_left_button_up();
	void on_right_button_down();
	void on_mouse_move();
	void on_key_down();

	void on_update();

	struct HoverData {
		ShaderNode *node;
		int port_in, port_out;
		int param;
		HoverData();
	};
	HoverData hover;
	HoverData get_hover();

	DataMaterial *data;
	ShaderGraph *graph;

	ShaderNode *node_moving;
	int move_dx, move_dy;

	struct NewLinkData {
		ShaderNode *node;
		int port;
		bool is_source;
		NewLinkData();
	};
	NewLinkData new_link;

	hui::Menu *popup;
};

#endif /* SRC_MODE_MATERIAL_DIALOG_SHADERGRAPHDIALOG_H_ */
