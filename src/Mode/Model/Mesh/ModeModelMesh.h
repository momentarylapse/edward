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

class Mode;
class DataModel;

class ModeModelMesh: public Mode
{
public:
	ModeModelMesh(Mode *_parent, DataModel *_data);
	virtual ~ModeModelMesh();

	void OnStart();
	void OnEnter();
	void OnEnd();

	void OnCommand(const string &id);
	void OnUpdate(Observable *o);
	void OnUpdateMenu();

	void OnDraw();

	void OptimizeView();

	void Easify();

	void CreateNewMaterialForSelection();
	void ChooseMaterialForSelection();

	void ChooseRightMouseFunction(int f);
	void ApplyRightMouseFunction(MultiView *mv);

	void AddEffects(int type);
	void EditEffects();
	void ClearEffects();

	DataModel *data;
	Data *GetData(){	return data;	}

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
