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
#include "../../../Data/Model/Geometry/ModelGeometry.h"
#include "../Dialog/ModelMaterialSelectionDialog.h"

class DataModel;

class ModeModelMesh: public Mode<DataModel>
{
public:
	ModeModelMesh(ModeBase *parent);
	virtual ~ModeModelMesh();

	virtual void OnStart();
	virtual void OnEnter();
	virtual void OnEnd();

	virtual void OnCommand(const string &id);
	virtual void OnUpdate(Observable *o);
	virtual void OnUpdateMenu();

	virtual void OnDraw();

	virtual bool OptimizeView();

	void Easify();

	void CreateNewMaterialForSelection();
	void ChooseMaterialForSelection();

	void ChooseRightMouseFunction(int f);
	void ApplyRightMouseFunction(MultiView *mv);

	void AddEffects(int type);
	void EditEffects();
	void ClearEffects();

	ModelMaterialSelectionDialog *MaterialSelectionDialog;

	enum{
		RMFRotate,
		RMFScale,
		RMFScale2d,
		RMFMirror
	};
	int right_mouse_function;

	void Copy();
	void Paste();
	bool Copyable();
	bool Pasteable();
	ModelGeometry TempGeo;
};

extern ModeModelMesh *mode_model_mesh;

#endif /* MODEMODELMESH_H_ */
