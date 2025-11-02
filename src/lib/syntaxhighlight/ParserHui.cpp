/*
 * ParserHui.cpp
 *
 *  Created on: 27.03.2021
 *      Author: michi
 */

#include "ParserHui.h"

ParserHui::ParserHui() : Parser("Hui") {
	macro_begin = "#";
	line_comment_begin = "//";
	multi_comment_begin = "/*";
	multi_comment_end = "*/";
	keywords.add("width");
	keywords.add("height");
	keywords.add("expandx");
	keywords.add("expandy");
	keywords.add("noexpandx");
	keywords.add("noexpandy");
	keywords.add("bold");
	keywords.add("italic");
	keywords.add("disabled");
	keywords.add("right");
	keywords.add("flat");
	keywords.add("big");
	keywords.add("huge");
	keywords.add("small");
	keywords.add("image");
	keywords.add("vertical");
	keywords.add("horizontal");
	keywords.add("range");
	keywords.add("margin");
	keywords.add("marginleft");
	keywords.add("marginright");
	keywords.add("margintop");
	keywords.add("marginbottom");
	keywords.add("padding");
	keywords.add("hgroup");
	keywords.add("vgroup");
	keywords.add("indent");
	keywords.add("noindent");
	keywords.add("grabfocus");
	keywords.add("ignorefocus");
	keywords.add("link");
	keywords.add("linked");
	keywords.add("box");
	keywords.add("danger");
	keywords.add("margin");
	keywords.add("alpha");
	keywords.add("editable");
	keywords.add("opengl");
	keywords.add("clearplaceholder");
	keywords.add("clearcompletion");
	keywords.add("placeholder");
	keywords.add("completion");
	keywords.add("actionbar");
	keywords.add("wrap");
	keywords.add("nowrap");
	keywords.add("center");
	keywords.add("angle");
	keywords.add("multiline");
	keywords.add("selectmulti");
	keywords.add("reorderable");
	keywords.add("format");
	keywords.add("noframe");
	keywords.add("handlekeys");
	keywords.add("monospace");
	keywords.add("tabsize");
	keywords.add("slide");
	keywords.add("up");
	keywords.add("down");
	keywords.add("left");
	keywords.add("right");
	keywords.add("crossfade");
	keywords.add("scrollx");
	keywords.add("scrolly");
	keywords.add("origin");
	keywords.add("showvalue");
	keywords.add("mark");
	keywords.add("clearmarks");
	keywords.add("important");
	keywords.add("checkable");
	keywords.add("style");
	keywords.add("text");
	keywords.add("icons");
	keywords.add("both");
	keywords.add("default");
	keywords.add("homogenousx");
	keywords.add("buttonbar");
	keywords.add("headerbar");
	keywords.add("closebutton");
	keywords.add("resizable");
	keywords.add("closable");
	keywords.add("cursor");
	keywords.add("borderwidth");
	keywords.add("yes");
	keywords.add("no");
	keywords.add("true");
	keywords.add("false");
	keywords.add("bar");
	keywords.add("nobar");
	keywords.add("buttonbar");
	keywords.add("switch");
	types.add("Dialog");
	types.add("Window");
	types.add("Menu");
	types.add("Toolbar");
	types.add("Separator");
	types.add("Label");
	types.add("Group");
	types.add("Button");
	types.add("Grid");
	types.add("Edit");
	types.add("MultilineEdit");
	types.add("ColorButton");
	types.add("CheckBox");
	types.add("RadioButton");
	types.add("DrawingArea");
	types.add("ComboBox");
	types.add("SpinButton");
	types.add("Slider");
	types.add("ProgressBar");
	types.add("TabControl");
	types.add("ListView");
	types.add("TreeView");
	types.add("Item");
	types.add("Separator");
	types.add("Scroller");
	types.add("ToggleButton");
	types.add("Paned");
	types.add("Revealer");
	types.add("Expander");
	types.add("MenuButton");
}

Array<Markup> ParserHui::create_markup(const string &text, int offset) {
	return create_markup_default(text, offset);
}
