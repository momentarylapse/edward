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

	virtual void onStart();
	virtual void onEnd();

	virtual void onCommand(const string &id);
	virtual void onUpdate(Observable *o, const string &message);

	virtual void onDraw();
	virtual void onDrawWin(MultiView::Window *win);

	virtual void onUpdateMenu();


	virtual void _new();
	virtual bool open();
	virtual bool save();
	virtual bool saveAs();

	virtual bool optimizeView();

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
