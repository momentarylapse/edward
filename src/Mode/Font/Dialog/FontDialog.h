/*
 * FontDialog.h
 *
 *  Created on: 20.03.2013
 *      Author: michi
 */

#ifndef FONTDIALOG_H_
#define FONTDIALOG_H_

#include "../../../lib/hui/hui.h"
#include "../../../Data/Font/DataFont.h"

class FontDialog: public hui::Panel
{
public:
	FontDialog(DataFont *_data);
	virtual ~FontDialog();

	void LoadData();
	void ApplyGlobalData();
	void ApplyGlyphData();

	void OnNum();
	void OnY1();
	void OnY2();
	void OnHeight();
	void OnFactorX();
	void OnFactorY();
	void OnUnknown();
	void OnName();
	void OnX1();
	void OnX2();
	void OnWidth();
	void OnText();

	string GetSampleText();

private:
	DataFont *data;
	DataFont::GlobalData global;
	DataFont::Glyph glyph;

	bool SettingData;
};

#endif /* FONTDIALOG_H_ */
