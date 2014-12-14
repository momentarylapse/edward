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
namespace Gui{
	class Font;
}

class ModeFont: public Mode<DataFont>
{
public:
	ModeFont();
	virtual ~ModeFont();

	virtual void onStart();
	virtual void onEnd();

	virtual void onMouseMove();
	virtual void onLeftButtonDown();
	virtual void onCommand(const string &id);
	virtual void onUpdate(Observable *o, const string &message);
	virtual void onUpdateMenu();

	virtual void onDraw();
	virtual void onDrawWin(MultiView::Window *win);


	virtual void _new();
	virtual bool open();
	virtual bool save();
	virtual bool saveAs();

	void Import();
	bool optimizeView();

	FontDialog *dialog;
	Gui::Font *font;
};

extern ModeFont* mode_font;

#endif /* MODEFONT_H_ */
