#include "BaseParser.h"
#include "Theme.h"
#include <lib/kapi/KabaExporter.h>

#include "AutoComplete.h"

void* get_theme() {
	return syntaxhighlight::default_theme;
}

using namespace syntaxhighlight;

void _export_package_syntaxhighlight_internal(kaba::IExporter* e) {
	e->declare_class_size("Markup", sizeof(Markup));
	e->declare_class_element("Markup.start", &Markup::start);
	e->declare_class_element("Markup.end", &Markup::end);
	e->declare_class_element("Markup.type", &Markup::type);

	e->declare_class_size("Parser.Label", sizeof(Parser::Label));
	e->declare_class_element("Parser.Label.category", &Parser::Label::category);
	e->declare_class_element("Parser.Label.name", &Parser::Label::name);
	e->declare_class_element("Parser.Label.line", &Parser::Label::line);
	e->declare_class_element("Parser.Label.level", &Parser::Label::level);

	e->declare_class_size("Parser.Error", sizeof(Parser::Error));
	e->declare_class_element("Parser.Error.filename", &Parser::Error::filename);
	e->declare_class_element("Parser.Error.message", &Parser::Error::message);
	e->declare_class_element("Parser.Error.position", &Parser::Error::position);

	e->declare_class_size("Parser.SymbolInfo", sizeof(Parser::SymbolInfo));
	e->declare_class_element("Parser.SymbolInfo.description", &Parser::SymbolInfo::description);
	e->declare_class_element("Parser.SymbolInfo.filename", &Parser::SymbolInfo::filename);
	e->declare_class_element("Parser.SymbolInfo.position", &Parser::SymbolInfo::position);
	e->declare_class_element("Parser.SymbolInfo.line", &Parser::SymbolInfo::line);

	e->declare_class_size("AutoCompleteData", sizeof(AutoCompleteData));
	e->declare_class_element("AutoCompleteData.offset", &AutoCompleteData::offset);
	e->declare_class_element("AutoCompleteData.suggestions", &AutoCompleteData::suggestions);

	e->declare_class_size("AutoCompleteData.Item", sizeof(AutoCompleteData::Item));
	e->declare_class_element("AutoCompleteData.Item.name", &AutoCompleteData::Item::name);
	e->declare_class_element("AutoCompleteData.Item.context", &AutoCompleteData::Item::context);

	Parser p("");
	e->declare_class_size("Parser", sizeof(Parser));
	e->link_virtual("Parser.__delete__", &kaba::generic_virtual<Parser>::__delete__, &p);
	e->link_virtual("Parser.prepare_symbols", &Parser::prepare_symbols, &p);
	e->link_virtual("Parser.find_labels", &Parser::find_labels, &p);
	e->link_virtual("Parser.find_errors", &Parser::find_errors, &p);
	e->link_virtual("Parser.word_type", &Parser::word_type, &p);
	e->link_virtual("Parser.create_markup", &Parser::create_markup, &p);
	e->link_virtual("Parser.symbol_info", &Parser::symbol_info, &p);
	e->link_virtual("Parser.run_autocomplete", &Parser::run_autocomplete, &p);

	e->link_func("create_parser", &create_parser);

	e->declare_class_size("ThemeContext", sizeof(syntaxhighlight::Context));
	e->declare_class_element("ThemeContext.fg", &syntaxhighlight::Context::fg);
	e->declare_class_element("ThemeContext.bg", &syntaxhighlight::Context::bg);
	e->declare_class_element("ThemeContext.bold", &syntaxhighlight::Context::bold);
	e->declare_class_element("ThemeContext.italic", &syntaxhighlight::Context::italic);
	e->declare_class_element("ThemeContext.set_bg", &syntaxhighlight::Context::set_bg);

	e->declare_class_size("Theme", sizeof(syntaxhighlight::Theme));
	e->declare_class_element("Theme.name", &syntaxhighlight::Theme::name);
	e->declare_class_element("Theme.context", &syntaxhighlight::Theme::context);
	e->declare_class_element("Theme.bg", &syntaxhighlight::Theme::bg);

	//e->link("theme", &syntaxhighlight::default_theme);
	e->link_func("get_theme", &::get_theme);
}

void export_package_syntaxhighlight(kaba::IExporter* e) {
	e->package_info("syntaxhighlight", "0.3");
	_export_package_syntaxhighlight_internal(e);
}
