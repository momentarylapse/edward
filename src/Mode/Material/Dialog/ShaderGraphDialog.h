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
	void on_mouse_move();

	DataMaterial *data;
	ShaderGraph *graph;
};

#endif /* SRC_MODE_MATERIAL_DIALOG_SHADERGRAPHDIALOG_H_ */
