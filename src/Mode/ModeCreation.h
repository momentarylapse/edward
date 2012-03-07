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

	virtual void Start() = 0;
	virtual void End() = 0;

	virtual void OnMouseMove() = 0;
	virtual void OnLeftButtonDown() = 0;
	virtual void OnLeftButtonUp() = 0;
	virtual void OnMiddleButtonDown() = 0;
	virtual void OnMiddleButtonUp() = 0;
	virtual void OnRightButtonDown() = 0;
	virtual void OnRightButtonUp() = 0;
	virtual void OnKeyDown() = 0;
	virtual void OnKeyUp() = 0;
	virtual void OnCommand(const string &id) = 0;

	virtual void PostDraw() = 0;
	virtual void PostDrawWin(int win, irect dest) = 0;

	string message;
	CHuiWindow *dialog;

private:
	// dummy
	void OnDataChange();
	void Draw();
	void DrawWin(int win, irect dest);
};

#endif /* MODECREATION_H_ */
