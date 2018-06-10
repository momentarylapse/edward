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

	void onStart() override;
	void onEnd() override;

	void onCommand(const string &id) override;
	void onUpdate(Observable *o, const string &message) override;

	void onDraw() override;
	void onDrawWin(MultiView::Window *win) override;

	void onUpdateMenu() override;


	void _new() override;
	bool open() override;
	bool save() override;
	bool saveAs() override;

	bool optimizeView() override;

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
