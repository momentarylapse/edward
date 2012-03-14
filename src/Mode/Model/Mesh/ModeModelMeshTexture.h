/*
 * ModeModelMeshTexture.h
 *
 *  Created on: 14.03.2012
 *      Author: michi
 */

#ifndef MODEMODELMESHTEXTURE_H_
#define MODEMODELMESHTEXTURE_H_

#include "../../Mode.h"
#include "../../../Data/Model/DataModel.h"

class Mode;
class DataModel;

class ModeModelMeshTexture: public Mode
{
public:
	ModeModelMeshTexture(Mode *_parent, DataModel *_data);
	virtual ~ModeModelMeshTexture();

	void Start();
	void End();

	void OnMouseMove();
	void OnLeftButtonDown();
	void OnLeftButtonUp();
	void OnMiddleButtonDown();
	void OnMiddleButtonUp();
	void OnRightButtonDown();
	void OnRightButtonUp();
	void OnKeyDown();
	void OnKeyUp();
	void OnCommand(const string &id);
	void OnUpdate(Observable *o);
	void OnUpdateMenu();

	void Draw();
	void DrawWin(int win, irect dest);

	DataModel *data;
};

extern ModeModelMeshTexture *mode_model_mesh_texture;

#endif /* MODEMODELMESHTEXTURE_H_ */
