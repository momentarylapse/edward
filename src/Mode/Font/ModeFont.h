/*
 * ModeFont.h
 *
 *  Created on: 08.03.2012
 *      Author: michi
 */

#ifndef MODEFONT_H_
#define MODEFONT_H_

#include "../Mode.h"
#include "../../Data/Font/DataFont.h"

class Mode;
class DataFont;
class FontDialog;

class ModeFont: public Mode
{
public:
	ModeFont();
	virtual ~ModeFont();

	void OnStart();
	void OnEnd();

	void OnMouseMove();
	void OnLeftButtonDown();
	void OnCommand(const string &id);
	void OnUpdate(Observable *o);
	void OnUpdateMenu();

	void OnDraw();
	void OnDrawWin(MultiViewWindow *win);


	void New();
	bool Open();
	bool Save();
	bool SaveAs();

	void Import();
	bool OptimizeView();

	DataFont *data;
	Data *GetData(){	return data;	}

	FontDialog *dialog;
};

extern ModeFont* mode_font;

#endif /* MODEFONT_H_ */
