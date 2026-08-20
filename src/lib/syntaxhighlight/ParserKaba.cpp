/*
 * ParserKaba.cpp
 *
 *  Created on: 06.06.2013
 *      Author: michi
 */

#include "ParserKaba.h"
#ifdef SYNTAX_HIGHLIGHT_KABA
#include "AutoComplete.h"
#include <lib/base/sort.h>
#include <lib/base/iter.h>
#include <lib/kaba/kaba.h>
#include <lib/kaba/parser/Parser.h>
#include <lib/os/file.h>
#include <lib/os/msg.h>

#include "lib/base/algo.h"


using namespace kaba;


namespace syntaxhighlight {

//static bool verbose = false;

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

CodeContext get_class_block_map(Module* m, const Class* c) {
	CodeContext bm{m, c, nullptr, nullptr, -1, -1};
	bm.start = m->tree->parser->Exp.token_logical_line(c->token_id)->offset;
	int last_token = c->token_id;
	for (const auto& e: c->elements) {
		if (c->parent and base::find_index_if(c->parent->elements, [e] (const ClassElement& ee) { return ee.name == e.name; }) >= 0)
			continue;
		last_token = max(last_token, e.token_id);
	}
	for (auto f: weak(c->functions)) {
		if (f->owner() == m->tree) // ignore inherited
			last_token = max(last_token, f->token_id);
	}
	for (auto cc: weak(c->constants))
		if (cc->owner == m->tree)
			last_token = max(last_token, cc->token_id);
	last_token = min(last_token, m->tree->parser->Exp.lines.back().last_token_id);
	auto l = m->tree->parser->Exp.token_logical_line(last_token);
	bm.end = l->offset + l->length;
	return bm;
}

Array<CodeContext> get_block_block_map(Module* m, Function* f, shared<Node> b) {
	struct Range { int first, last; };
	base::map<Block*, Range> block_tokens;
	auto mark = [&block_tokens] (Block* b, int token_id) {
		if (block_tokens.contains(b))
			block_tokens[b] = {min(block_tokens[b].first, token_id), max(block_tokens[b].last, token_id)};
		else
			block_tokens.set(b, {token_id, token_id});
	};

	// mark (direct) coverage of all blocks
	kaba::Transformer::transformb_block(b.get(), [mark, m] (shared<Node> n, Block* bb) {
		if (bb and n->token_id >= 0 and !bb->function->is_template() and !bb->function->auto_declared and bb->function->name.head(1) != ":" and bb->function->owner() == m->tree)
			mark(bb, n->token_id);
		return n;
	});

	// make parent cover children
	for (const auto [bb, range]: block_tokens) {
		auto bbb = bb;
		while (bbb->parent) {
			block_tokens[bbb->parent] = {min(block_tokens[bbb->parent].first, range.first), max(block_tokens[bbb->parent].last, range.last)};
			bbb = bbb->parent;
		}
	}

	// tokens -> offsets
	Array<CodeContext> map;
	for (const auto& [bb, range]: block_tokens) {
		if (auto l = m->tree->parser->Exp.token_logical_line(range.first)) {
			int first = l->offset;
			int last = m->tree->parser->Exp.token_offset(range.last) + m->tree->parser->Exp.get_token(range.last).num;
			CodeContext bm{m, f->name_space, f, bb, first, last};
			map.add(bm);
		}
	}
	return map;
}

Array<CodeContext> get_block_map(Module* m) {
	Array<CodeContext> map;

	//m->tree->root_node

	for (auto c: m->classes()) {
		map.add(get_class_block_map(m, c));
		for (auto cc: weak(c->classes))
			map.add(get_class_block_map(m, cc));
	}

	auto is_templated_class = [] (const Class* t) {
		static const Array<string> evil = {"[", "{", "&", "*", "?"};
		for (const auto& e: evil)
			if (t->name.find(e) >= 0)
				return true;
		return false;
	};

	for (auto f: m->tree->functions)
		if (f->owner() == m->tree and !f->auto_declared and !is_templated_class(f->name_space)) {
			CodeContext bm{m, f->name_space, f, f->block, -1, -1};
			bm.start = m->tree->parser->Exp.token_offset(f->token_id);
			int last_token = f->token_id;
			if (f->abstract_node)
				kaba::Transformer::transform_node(f->abstract_node.get(), [&last_token] (shared<Node> n) {
					last_token = max(last_token, n->token_id);
					return n;
				});
			bm.end = m->tree->parser->Exp.token_offset(last_token) + m->tree->parser->Exp.get_token(last_token).num;

			if (f->block_node)
				map.append(get_block_block_map(m, f, f->block_node));
			map.add(bm);
		}
	return map;
}

void ParserKaba::prepare_symbols(const string &text, const Path& filename) {
	if (text == current_code)
		return;

	current_code = text;
	errors.clear();

	//msg_write(kaba::config.directory.str());
	auto new_context = kaba::default_context->create_new_context();
	auto new_module = new_context->create_module_for_source(text, filename, CompilerFlags::JustParse);
	if (new_module) {

		clear_symbols();

		module = *new_module;
		context = new_context;

		block_map = get_block_map(module.get());

	} else {

		// read errors
		for (const auto& e: new_context->get_errors()) {
			const auto& l = e.locations.back();
			int offset = 0;
			if (e.locations.num == 1) {
				offset = text_line_column_to_offset(text, l.line, l.column);
			} else {
				try {
					offset = text_line_column_to_offset(os::fs::read_text(l.filename), l.line, l.column);
				} catch (...) {}
			}
			errors.add({l.filename, e.message, offset});
		}
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

CodeContext ParserKaba::guess_context(int offset) {
	if (!module)
		return {};

	CodeContext x;
	x.module = module.get();
	x.c = module->base_class();
	x.start = 0;
	x.end = 9999999;

	for (const auto& bm: block_map)
		if (bm.start <= offset and bm.end >= offset)
			if (bm.end - bm.start < x.end - x.start) // prefer smallest (= inner most)
				x = bm;

	return x;
}

MarkupType node_to_markup(shared<Node> n) {
	if (n->kind == kaba::NodeKind::Module or n->kind == kaba::NodeKind::Class)
		return MarkupType::TYPE;
	if (n->kind == kaba::NodeKind::Function)
		return MarkupType::COMPILER_FUNCTION;
	if (n->kind == kaba::NodeKind::Constant)
		return MarkupType::GLOBAL_VARIABLE;
	if (n->kind == kaba::NodeKind::VarGlobal)
		return MarkupType::GLOBAL_VARIABLE;
	if (n->kind == kaba::NodeKind::VarLocal)
		return MarkupType::LOCAL_VARIABLE;
	if (n->kind == kaba::NodeKind::ClassElement)
		return MarkupType::LOCAL_VARIABLE;
	if (n->kind == kaba::NodeKind::AddressShift or n->kind == kaba::NodeKind::DereferenceAddressShift)
		// member variable (self.x)
		return MarkupType::LOCAL_VARIABLE;
//	n->show();
	return MarkupType::WORD;
}

Array<Markup> ParserKaba::create_markup(const string &text, int offset) {
	auto markup = create_markup_default(text, offset);

	if (!module)
		return markup;

	shared<kaba::Node> parent_node;

	for (auto& m: markup) {

		string temp;
		if (m.end - m.start < 64)
			temp = text.sub_ref(m.start - offset, m.end - offset);

		if (temp == ".")
			continue;

		if constexpr (false) {
			auto xx = guess_context(m.start);
			m.type = (MarkupType)((xx.start + 4) % (int)MarkupType::NUM_TYPES);
			continue;
		}

		if (m.type == MarkupType::WORD) {

			if (parent_node) {
				auto e = module->tree->get_element_of(parent_node, temp, -1);
				if (e.num >= 1) {
					m.type = node_to_markup(e[0]);
					parent_node = e[0];
				} else {
					msg_write(temp);
					parent_node->show();
					parent_node = nullptr;
				}

			} else {
				auto x = guess_context(m.start);
				//m.type = (MarkupType)(((int_p)x.f >> 8) % (int)MarkupType::NUM_TYPES);
				auto e = module->tree->get_existence(temp, x.b, x.c, -1);
				if (e.num >= 1) {
					m.type = node_to_markup(e[0]);
					parent_node = e[0];
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

	auto c = guess_context(offset);
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
	} else if (n->kind == kaba::NodeKind::AddressShift or n->kind == kaba::NodeKind::DereferenceAddressShift) {
		auto t0 = simplify_type(n->params[0]->type);
		if (auto i = find_class_element(t0, (int)n->link_no))
			return xxx(i->module, i->token_id, format("element  %s: %s", i->name, i->type->long_name()));
		return xxx(t0->owner->module, t0->token_id, format("element  %s: %s", "???", n->type->long_name()));
	} else if (n->kind == kaba::NodeKind::ClassElement) {
		auto e = (ClassElement*)(int_p)n->link_no;
		return xxx(ci.module, e->token_id, format("element  %s: %s", e->name, e->type->long_name()));
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
	auto c = guess_context(offset);
	if (auto node = parse_node(c, terms))
		return node_info(c, node.get());

	return base::None;
}

}
#endif
