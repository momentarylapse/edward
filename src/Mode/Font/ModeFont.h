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

class ModeFont: public Mode
{
public:
	ModeFont();
	virtual ~ModeFont();

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


	void New();
	bool Open();
	bool Save();
	bool SaveAs();

	DataFont *data;
};

extern ModeFont* mode_font;

#endif /* MODEFONT_H_ */
