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

class ShaderGraphDialog : public hui::Panel {
public:
	ShaderGraphDialog(DataMaterial *data);
	virtual ~ShaderGraphDialog();

	void draw_node(Painter *p, ShaderNode *n);

	void on_draw(Painter *p);

	void on_left_button_down();
	void on_left_button_up();
	void on_right_button_down();
	void on_mouse_move();
	void on_mouse_wheel();
	void on_key_down();

	void on_update();
	void on_reset();

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

	void update_mouse();
	float mx, my;
	int move_dx, move_dy;
	float view_scale;
	float view_offset_x, view_offset_y;
	float proj_x(float x);
	float proj_y(float y);
	rect proj_rect(const rect &r);


	hui::Menu *popup;
};

#endif /* SRC_MODE_MATERIAL_DIALOG_SHADERGRAPHDIALOG_H_ */
