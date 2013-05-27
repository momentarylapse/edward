/*
 * ModeWelcome.h
 *
 *  Created on: 04.03.2012
 *      Author: michi
 */

#ifndef MODEWELCOME_H_
#define MODEWELCOME_H_

#include "../Mode.h"

class ModeWelcome: public ModeBase {
public:
	ModeWelcome();
	virtual ~ModeWelcome();

	virtual void OnStart();
	virtual void OnEnd();

	virtual void OnMouseMove();
	virtual void OnLeftButtonUp();
	virtual void OnCommand(const string &id);
	virtual void OnUpdate(Observable *o);

	virtual void OnDraw();

	virtual Data *GetData(){	return NULL;	};

	void DrawIcon(int x, int y, int index, int pic_index, const string &name);

	int tex_icons, tex_logo;
	rect icon_rect[7];
	int mouse_over, mouse_over_cmd;
};

extern ModeWelcome *mode_welcome;

#endif /* MODEWELCOME_H_ */
