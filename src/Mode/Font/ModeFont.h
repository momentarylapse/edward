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

class FontDialog;

class ModeFont: public Mode<DataFont>
{
public:
	ModeFont();
	virtual ~ModeFont();

	virtual void OnStart();
	virtual void OnEnd();

	virtual void OnMouseMove();
	virtual void OnLeftButtonDown();
	virtual void OnCommand(const string &id);
	virtual void OnUpdate(Observable *o);
	virtual void OnUpdateMenu();

	virtual void OnDraw();
	virtual void OnDrawWin(MultiViewWindow *win);


	virtual void New();
	virtual bool Open();
	virtual bool Save();
	virtual bool SaveAs();

	void Import();
	bool OptimizeView();

	FontDialog *dialog;
};

extern ModeFont* mode_font;

#endif /* MODEFONT_H_ */
