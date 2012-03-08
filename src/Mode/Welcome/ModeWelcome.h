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

	void Draw();
	void DrawWin(int win, irect dest);

	void DrawIcon(int x, int y, int index, const string &name);

	int tex_icons, tex_logo;
	irect icon_rect[7];
	int mouse_over, mouse_over_cmd;
};

extern ModeWelcome *mode_welcome;

#endif /* MODEWELCOME_H_ */
