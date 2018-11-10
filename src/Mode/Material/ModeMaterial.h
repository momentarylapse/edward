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
class Geometry;
namespace nix{
	class VertexBuffer;
};

class ModeMaterial: public Mode<DataMaterial>
{
public:
	ModeMaterial();
	virtual ~ModeMaterial();

	void on_start() override;
	void on_end() override;

	void on_command(const string &id) override;
	void on_update(Observable *o, const string &message) override;

	void on_draw() override;
	void on_draw_win(MultiView::Window *win) override;

	void on_update_menu() override;


	void _new() override;
	bool open() override;
	bool save() override;
	bool save_as() override;

	bool optimize_view() override;

	nix::VertexBuffer *MaterialVB[MATERIAL_MAX_TEXTURES + 1];
	MaterialPropertiesDialog *AppearanceDialog;

	string shape_type;
	bool shape_smooth;
	Geometry *geo;

	void SetShapeType(const string &type);
	void SetShapeSmooth(bool smooth);
	void UpdateShape();
};

extern ModeMaterial* mode_material;

#endif /* MODEMATERIAL_H_ */
