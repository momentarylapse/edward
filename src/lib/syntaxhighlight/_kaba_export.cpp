#include "BaseParser.h"
#include "Theme.h"
#include <lib/kapi/KabaExporter.h>

void* get_theme() {
	return syntaxhighlight::default_theme;
}

void _export_package_syntaxhighlight_internal(kaba::IExporter* e) {
	e->declare_class_size("Markup", sizeof(Markup));
	e->declare_class_element("Markup.start", &Markup::start);
	e->declare_class_element("Markup.end", &Markup::end);
	e->declare_class_element("Markup.type", &Markup::type);

	e->declare_class_size("Parser.Label", sizeof(Parser::Label));
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

	Parser p("");
	e->declare_class_size("Parser", sizeof(Parser));
	e->link_virtual("Parser.prepare_symbols", &Parser::prepare_symbols, &p);
	e->link_virtual("Parser.find_labels", &Parser::find_labels, &p);
	e->link_virtual("Parser.find_errors", &Parser::find_errors, &p);
	e->link_virtual("Parser.word_type", &Parser::word_type, &p);
	e->link_virtual("Parser.create_markup", &Parser::create_markup, &p);
	e->link_virtual("Parser.symbol_info", &Parser::symbol_info, &p);

	e->link_func("get_parser", &GetParser);

	e->declare_class_size("Context", sizeof(syntaxhighlight::Context));
	e->declare_class_element("Context.fg", &syntaxhighlight::Context::fg);
	e->declare_class_element("Context.bg", &syntaxhighlight::Context::bg);
	e->declare_class_element("Context.bold", &syntaxhighlight::Context::bold);
	e->declare_class_element("Context.italic", &syntaxhighlight::Context::italic);
	e->declare_class_element("Context.set_bg", &syntaxhighlight::Context::set_bg);

	e->declare_class_size("Theme", sizeof(syntaxhighlight::Theme));
	e->declare_class_element("Theme.name", &syntaxhighlight::Theme::name);
	e->declare_class_element("Theme.context", &syntaxhighlight::Theme::context);
	e->declare_class_element("Theme.bg", &syntaxhighlight::Theme::bg);

	//e->link("theme", &syntaxhighlight::default_theme);
	e->link_func("get_theme", &get_theme);
}

void export_package_syntaxhighlight(kaba::IExporter* e) {
	e->package_info("syntaxhighlight", "0.1");
	_export_package_syntaxhighlight_internal(e);
}
