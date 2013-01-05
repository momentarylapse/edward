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

	void OnStart();
	void OnEnd();

	void OnMouseMove();
	void OnLeftButtonUp();
	void OnCommand(const string &id);
	void OnUpdate(Observable *o);

	void OnDraw();

	void DrawIcon(int x, int y, int index, int pic_index, const string &name);

	int tex_icons, tex_logo;
	rect icon_rect[7];
	int mouse_over, mouse_over_cmd;
};

extern ModeWelcome *mode_welcome;

#endif /* MODEWELCOME_H_ */
