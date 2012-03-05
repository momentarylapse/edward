/*
 * ModeWelcome.h
 *
 *  Created on: 04.03.2012
 *      Author: michi
 */

#ifndef MODEWELCOME_H_
#define MODEWELCOME_H_

#include "../Mode.h"

class ModeWelcome: public Mode {
public:
	ModeWelcome();
	virtual ~ModeWelcome();

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
	virtual void OnDataChange();

	virtual void Draw();

	void DrawIcon(int x, int y, int index, const string &name);

	int tex_icons, tex_logo;
	irect icon_rect[7];
	int mouse_over, mouse_over_cmd;
};

extern ModeWelcome *mode_welcome;

#endif /* MODEWELCOME_H_ */
