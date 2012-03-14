/*
 * ModeCreation.h
 *
 *  Created on: 07.03.2012
 *      Author: michi
 */

#ifndef MODECREATION_H_
#define MODECREATION_H_

#include "Mode.h"
#include "../lib/file/file.h"

class ModeCreation: public Mode
{
public:
	ModeCreation();
	virtual ~ModeCreation();

	virtual void Start();
	virtual void End();

	virtual void OnMouseMove();
	virtual void OnLeftButtonDown();
	virtual void OnLeftButtonUp();
	virtual void OnMiddleButtonDown();
	virtual void OnMiddleButtonUp();
	virtual void OnRightButtonDown();
	virtual void OnRightButtonUp();
	virtual void OnKeyDown();
	virtual void OnKeyUp();
	virtual void OnCommand(const string &id);
	virtual void OnUpdateMenu();

	virtual void PostDraw();
	virtual void PostDrawWin(int win, irect dest);

	string message;
	CHuiWindow *dialog;

private:
	// dummy
	void OnUpdate(Observable *o);
	void Draw();
	void DrawWin(int win, irect dest);
};

#endif /* MODECREATION_H_ */
