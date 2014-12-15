/*
 * ModeModelMesh.h
 *
 *  Created on: 04.03.2012
 *      Author: michi
 */

#ifndef MODEMODELMESH_H_
#define MODEMODELMESH_H_

#include "../../Mode.h"
#include "../../../Data/Model/DataModel.h"
#include "../../../Data/Model/Geometry/Geometry.h"

class DataModel;
class ModelMaterialDialog;

class ModeModelMesh: public Mode<DataModel>, public Observable
{
public:
	ModeModelMesh(ModeBase *parent);
	virtual ~ModeModelMesh();

	virtual void onStart();
	virtual void onEnter();
	virtual void onEnd();

	virtual void onCommand(const string &id);
	virtual void onUpdate(Observable *o, const string &message);
	virtual void onUpdateMenu();

	virtual void onDraw();

	virtual bool optimizeView();

	void easify();

	void createNewMaterialForSelection();
	void chooseMaterialForSelection();

	void chooseMouseFunction(int f);
	void applyMouseFunction(MultiView::MultiView *mv);

	void addEffects(int type);
	void editEffects();
	void clearEffects();

	ModelMaterialDialog *material_dialog;
	void showMaterialDialog();
	void closeMaterialDialog();
	void toggleMaterialDialog();

	int mouse_action;

	void copy();
	void paste();
	bool copyable();
	bool pasteable();
	Geometry temp_geo;


	int current_material;
	void setCurrentMaterial(int index);
};

extern ModeModelMesh *mode_model_mesh;

#endif /* MODEMODELMESH_H_ */
