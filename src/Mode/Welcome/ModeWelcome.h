/*
 * ModeWelcome.h
 *
 *  Created on: 04.03.2012
 *      Author: michi
 */

#ifndef MODEWELCOME_H_
#define MODEWELCOME_H_

#include "../Mode.h"

class NixTexture;

class ModeWelcome: public ModeBase {
public:
	ModeWelcome();
	virtual ~ModeWelcome();

	virtual void onStart();
	virtual void onEnd();

	virtual void onMouseMove();
	virtual void onLeftButtonUp();
	virtual void onCommand(const string &id);
	virtual void onUpdate(Observable *o, const string &message);

	virtual void onDraw();

	virtual Data *getData(){	return NULL;	};

	void DrawIcon(int x, int y, int index, int pic_index, const string &name);

	NixTexture *tex_icons, *tex_logo;
	rect icon_rect[7];
	int mouse_over, mouse_over_cmd;
};

extern ModeWelcome *mode_welcome;

#endif /* MODEWELCOME_H_ */
