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

	void on_start() override;
	void on_end() override;

	void on_mouse_move() override;
	void on_left_button_down() override;
	void on_command(const string &id) override;
	void on_update(Observable *o, const string &message) override;
	void on_update_menu() override;

	void on_draw() override;
	void on_draw_win(MultiView::Window *win) override;


	void _new() override;
	bool open() override;
	bool save() override;
	bool save_as() override;

	void Import();
	bool optimize_view();

	FontDialog *dialog;
	Gui::Font *font;
};

extern ModeFont* mode_font;

#endif /* MODEFONT_H_ */
