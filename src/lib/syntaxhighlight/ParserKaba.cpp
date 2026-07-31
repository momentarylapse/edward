/*
 * ParserKaba.cpp
 *
 *  Created on: 06.06.2013
 *      Author: michi
 */

#include "ParserKaba.h"
#include "AutoComplete.h"
//#ifdef SYNTAX_HIGHLIGHT_KABA ...
#include <lib/kaba/kaba.h>
#include <lib/kaba/parser/Parser.h>
#include <lib/base/iter.h>
#include <lib/os/file.h>
#include <stdio.h>

#include "lib/base/sort.h"
#include "lib/os/msg.h"

using namespace kaba;

	const kaba::Class *simplify_type(const kaba::Class *c) {
		if (c->is_some_pointer_not_null())
			return c->param[0];
		return c;
	}

	const kaba::Class *node_namespace(shared<kaba::Node> n) {
		if (n->kind == NodeKind::Class)
			return n->as_class();
		return simplify_type(n->type);
	}

namespace syntaxhighlight {

static bool verbose = false;

bool allowed(const string &s) {
	if (s == "filename" or s == "config")
		return false;
	return true;
}

ParserKaba::ParserKaba() : Parser("Kaba") {
	macro_begin = "#define";
	line_comment_begin = "#";
	string_sub_begin = "{{";
	string_sub_end = "}}";
	keywords.append(kaba::default_context->list_keywords());
	modifiers.append(kaba::default_context->list_modifiers());
	compiler_functions.append(kaba::default_context->list_special_functions());
	operator_functions.append(kaba::default_context->list_operator_functions());

	//for (auto &s: kaba::Statements)
	//	special_words.add(s.name);
}

void ParserKaba::clear_symbols() {
	types.clear();
	global_variables.clear();
	functions.clear();
	constants.clear();
}

int text_line_column_to_offset(const string& text, int line, int col) {
	int offset = col;
	auto lines = text.explode("\n");
	for (int i=0; i<min(line, lines.num); i++)
		offset += lines[i].num + 1;
	return offset;
}

void ParserKaba::prepare_symbols(const string &text, const Path& filename) {
	if (text == current_code)
		return;

	current_code = text;
	errors.clear();

	try {
		//msg_write(kaba::config.directory.str());
		auto new_context = kaba::default_context->create_new_context();
		auto new_module = new_context->create_module_for_source(text, filename, true);

		clear_symbols();

		module = new_module;
		context = new_context;

	} catch (kaba::Exception &e) {
		auto ee = &e;
		while (ee->parent)
			ee = ee->parent.get();

		int offset = 0;
		if (ee == &e) {
			offset = text_line_column_to_offset(text, ee->line, ee->column);
		} else {
			try {
				offset = text_line_column_to_offset(os::fs::read_text(ee->filename), ee->line, ee->column);
			} catch (...) {}
		}
		errors.add({ee->filename, ee->message(), offset});
	} catch (::Exception &e) {
		errors.add({"", e.message(), 0});
		//msg_error(e.message());
	}
}


Array<Parser::Label> ParserKaba::find_labels(const string& text) {
	Array<Label> labels;

	auto ff = [] (const string& s) {
		auto x = s.replace(" virtual ", " ").replace(" extern ", " ").replace(" mut ", " ").replace(" selfref ", " ").replace(" globalref ", " ").replace(" pure ", " ").replace(" override ", " ").replace(" static ", " ").replace(" as Sharable", "").replace(" as @noauto", "");
		int p = x.find(" extends ");
		if (p > 0)
			return x.head(p);
		return x;
	};

	auto lines = text.explode("\n");
	string last_class;
	for (const auto& [line_no, l]: enumerate(lines)) {
		if (l.num < 4)
			continue;
		auto ll = l.trim();
		int level = 0;
		if (l[0] == '\t')
			level ++;
		if (l[1] == '\t')
			level ++;
		if (l[2] == '\t')
			level ++;
		// meh :P
		int p0 = ll.find(" ");
		if (p0 < 0)
			continue;
		string cat = ll.head(p0);
		if (cat == "class" or cat == "struct" or cat == "namespace" or cat == "enum" or cat == "trait" or cat == "func" or cat == "macro") {
			// sub(1)... we keep " " at the beginning for cleaner removal of " extern " etc
			labels.add({cat, ff(ll.sub(p0)).sub(1), line_no, level});
		}
	}
	return labels;
}

Array<Parser::Error> ParserKaba::find_errors(const string &text) {
	return errors;
}

struct CodeContext {
	Module* module = nullptr;
	const kaba::Class* c = nullptr;
	kaba::Function* f = nullptr;
	kaba::Block* b = nullptr;
};

CodeContext guess_context(kaba::Module* m, int offset) {
	if (!m)
		return {};

	CodeContext x;
	x.module = m;

	for (auto c: m->classes()) {
		int c_off = m->tree->parser->Exp.token_offset(c->token_id);
		if (c_off >= 0 and c_off <= offset) {
			x.c = c;
		}
	}

	for (auto f: m->tree->functions) {
		int f_off = m->tree->parser->Exp.token_offset(f->token_id);
		if (!f->is_extern() and !f->is_template() and !f->auto_declared and f_off >= 0 and f_off <= offset) {
			x.c = f->name_space;
			x.f = f;
			x.b = f->block;
		}
	}
	return x;
}

Array<Markup> ParserKaba::create_markup(const string &text, int offset) {
	auto markup = create_markup_default(text, offset);

	if (!module)
		return markup;

	shared<kaba::Node> parent_node;

	for (auto& m: markup) {

		string temp;
		if (m.end - m.start < 64)
			temp = text.sub_ref(m.start, m.end);

		if (temp == ".")
			continue;

		if (m.type == MarkupType::WORD) {

			if (parent_node) {
				auto e = module->tree->get_element_of(parent_node, temp, -1);
				if (e.num >= 1) {
					if (e[0]->kind == kaba::NodeKind::Class)
						m.type = MarkupType::TYPE;
					else if (e[0]->kind == kaba::NodeKind::Function)
						m.type = MarkupType::COMPILER_FUNCTION;
					else if (e[0]->kind == kaba::NodeKind::Constant)
						m.type = MarkupType::GLOBAL_VARIABLE;
					else if (e[0]->kind == kaba::NodeKind::AddressShift or e[0]->kind == kaba::NodeKind::DereferenceAddressShift)
						m.type = MarkupType::LOCAL_VARIABLE;
//					else
//						e[0]->show();
					parent_node = e[0];
				} else {
					msg_write(temp);
					parent_node->show();
					parent_node = nullptr;
				}

			} else {
				auto x = guess_context(module.get(), m.start);
				//		m.type = (MarkupType)(((int_p)x.f >> 8) % (int)MarkupType::NUM_TYPES);
				if (x.c) {
					auto e = module->tree->get_existence(temp, x.b, x.c, -1);
					if (e.num >= 1) {
						if (e[0]->kind == kaba::NodeKind::Module or e[0]->kind == kaba::NodeKind::Class)
							m.type = MarkupType::TYPE;
						else if (e[0]->kind == kaba::NodeKind::Function)
							m.type = MarkupType::COMPILER_FUNCTION;
						else if (e[0]->kind == kaba::NodeKind::Constant)
							m.type = MarkupType::GLOBAL_VARIABLE;
						else if (e[0]->kind == kaba::NodeKind::VarGlobal)
							m.type = MarkupType::GLOBAL_VARIABLE;
						else if (e[0]->kind == kaba::NodeKind::VarLocal)
							m.type = MarkupType::LOCAL_VARIABLE;
						else if (e[0]->kind == kaba::NodeKind::AddressShift)
							// member variable (self.x)
							m.type = MarkupType::LOCAL_VARIABLE;
//						else
//							e[0]->show();
						parent_node = e[0];
					}
				}
			}
		} else {
			parent_node = nullptr;
		}
	}
	return markup;
}




bool allow(const string &name) {
	if (name.head(1) == "-")
		return false;
	if (name.tail(1) == "?")
		return false;
	if (name.find(".") >= 0)
		return false;
	if (name.find("[") >= 0 or name.find("*") >= 0 or name.find("{") >= 0 or name.find("&") >= 0)
		return false;
	return true;
}

AutoCompleteData suggest_from_class(const kaba::Class *t, const string &yyy) {
	t = simplify_type(t);
	AutoCompleteData suggestions;
	suggestions.offset = yyy.num;
	for (auto &e: t->elements)
		if (e.name.head(yyy.num) == yyy and allow(e.name))
			suggestions.add(e.name, format("%s.%s: %s", t->name, e.name, e.type->name));
	for (auto f: t->functions)
		if (f->name.head(yyy.num) == yyy and allow(f->name))
			suggestions.add(f->name, f->signature());
	for (auto c: t->classes)
			if (c->name.head(yyy.num) == yyy and allow(c->name))
				suggestions.add(c->name, "class " + c->long_name());
	for (auto c: t->constants)
		if (c->name.head(yyy.num) == yyy and allow(c->name))
			suggestions.add(c->name, format("const %s: %s", c->name, c->type->long_name()));
	return suggestions;
}

AutoCompleteData find_top_level_from_scope(const Scope& s, const string& yyy) {
	AutoCompleteData suggestions;
	suggestions.offset = yyy.num;
	for (auto &e: s.entries)
		if (e.name.head(yyy.num) == yyy and allow(e.name)) {
			if (e.kind == NodeKind::Module)
				suggestions.add(e.name, "module " + e.name);
			if (e.kind == NodeKind::Class)
				suggestions.add(e.name, "class " + e.name);
			if (e.kind == NodeKind::Function)
				suggestions.add(e.name, "func " + e.name);
			if (e.kind == NodeKind::VarGlobal)
				suggestions.add(e.name, "var " + e.name);
			if (e.kind == NodeKind::Constant)
				suggestions.add(e.name, "const " + e.name);
		}
	return suggestions;
}

AutoCompleteData suggest_top_level(const CodeContext& c, const string &yyy) {
	AutoCompleteData suggestions;
	suggestions.offset = yyy.num;

	// general expressions
	for (string &e: c.module->context->list_keywords())
		if (yyy == e.head(yyy.num))
			suggestions.add(e, e);

	// function local
	if (c.f) {
		for (auto v: c.f->var)
			if (yyy == v->name.head(yyy.num) and allow(v->name))
				suggestions.add(v->name, v->type->name + " " + v->name);
	}
	if (c.c)
		suggestions.append(suggest_from_class(c.c, yyy));

	suggestions.append(find_top_level_from_scope(c.module->tree->global_scope, yyy));

	return suggestions;
}




Array<string> parse_simple_backwards(const string& code, int offset) {
	Array<string> terms;

	for (int i=offset-1; i>=0; i--) {
		if (code[i] == '.' or is_spacing((char)code[i]) or is_sign((char)code[i])) {
			terms.insert(code.sub(i+1, offset), 0);
			offset = i;
			if (code[i] == '.')
				terms.insert(".", 0);
			else
				break;
		}
	}
	return terms;
}

AutoCompleteData suggest_child(const CodeContext& c, Node* parent, const string& name) {
	auto type = simplify_type(parent->type);
	if (parent->kind == NodeKind::Module)
		type = parent->as_module()->tree->base_class;
	if (parent->kind == NodeKind::Class)
		type = parent->as_class();
	return suggest_from_class(type, name);
}

AutoCompleteData xsort(const AutoCompleteData& a) {
	auto r = a;
	r.suggestions = base::sorted(r.suggestions, [] (const AutoCompleteData::Item& a, const AutoCompleteData::Item& b) {
		return a.name <= b.name;
	});
	for (int i=1; i<r.suggestions.num; i++)
		if (r.suggestions[i].name == r.suggestions[i-1].name)
			r.suggestions.erase(i--);
	return r;
}


shared<Node> parse_node(const CodeContext& c, const Array<string>& terms) {
	shared<Node> node;

	if (c.c) {
		if (terms.num >= 1) {
			// root
			auto e = c.module->tree->get_existence(terms[0], c.b, c.c, -1);
			if (e.num >= 1)
				node = e[0];
		}
		for (int i=2; i<terms.num; i+=2)
			if (node) {
				auto e = c.module->tree->get_element_of(node, terms[i], -1);
				if (e.num >= 1)
					node = e[0];
			}
	}
	return node;
}

AutoCompleteData ParserKaba::run_autocomplete(const string& code, const Path& filename, int offset) {

	auto terms = parse_simple_backwards(code, offset);
	//msg_write(str(terms));

	auto c = guess_context(module.get(), offset);
	if (terms.num >= 3) {
		if (shared<Node> node = parse_node(c, terms.sub_ref(0, -2)))
			return xsort(suggest_child(c, node.get(), terms.back()));
	} else {
		return xsort(suggest_top_level(c, terms[0]));
	}

	return {};
}

struct ElementInfo {
	Module* module;
	int token_id;
	string name;
	const Class* type;
};

base::optional<ElementInfo> find_class_element(const Class* t0, int offset) {
	if (t0->parent)
		if (auto i = find_class_element(t0->parent, offset))
			return i;

	for (const auto& e: t0->elements)
		if (e.offset == offset)
			return ElementInfo{t0->owner->module, e.token_id, e.name, e.type};
	return base::None;
}

base::optional<Parser::SymbolInfo> node_info(const CodeContext& ci, kaba::Node* n) {
	auto xxx = [] (const kaba::Module* m, int token_id, const string& description) -> Parser::SymbolInfo {
		Parser::SymbolInfo o;
		o.description = description;
		if (m->tree and m->tree->parser) {
			o.filename = m->filename;
			o.position = m->tree->parser->Exp.token_offset(token_id);
			o.line = m->tree->parser->Exp.token_physical_line_no(token_id);
			return o;
		}
		return o;
	};

	//n->show();
	if (n->kind == kaba::NodeKind::Module) {
		return xxx(n->as_module(), 0, "module");
	} else if (n->kind == kaba::NodeKind::Class) {
		auto t = n->as_class();
		return xxx(t->owner->module, t->token_id, "class");
	} else if (n->kind == kaba::NodeKind::Function) {
		auto f = n->as_func();
		return xxx(f->owner()->module, f->token_id, "function  " + f->signature());
	} else if (n->kind == kaba::NodeKind::Constant) {
		auto c = n->as_const();
		return xxx(c->owner->module, c->token_id, format("constant  %s: %s = %s", c->name, c->type->long_name(), c->str()));
	} else if (n->kind == kaba::NodeKind::VarGlobal) {
		auto v = n->as_global();
		return xxx(v->ns->owner->module, v->token_id, format("variable  %s: %s", v->name, v->type->long_name()));
	} else if (n->kind == kaba::NodeKind::VarLocal) {
		auto v = n->as_local();
		return xxx(ci.module, v->token_id, format("variable  %s: %s", v->name, v->type->long_name()));
	} else if (n->kind == kaba::NodeKind::AddressShift) {
		auto t0 = n->params[0]->type;
		if (auto i = find_class_element(t0, (int)n->link_no))
			return xxx(i->module, i->token_id, format("element  %s: %s", i->name, i->type->long_name()));
		return xxx(t0->owner->module, t0->token_id, format("element  %s: %s", "???", n->type->long_name()));
	} else if (n->kind == kaba::NodeKind::Statement) {
		return xxx(nullptr, -1, "statement");
	}
	return base::None;
}

base::optional<Parser::SymbolInfo> ParserKaba::symbol_info(const string& text, int offset, int length) {
	if (!module)
		return base::None;

	auto terms = parse_simple_backwards(text, offset + length);
	//msg_write(str(terms));
	auto c = guess_context(module.get(), offset);
	if (auto node = parse_node(c, terms))
		return node_info(c, node.get());

	return base::None;
}

}
