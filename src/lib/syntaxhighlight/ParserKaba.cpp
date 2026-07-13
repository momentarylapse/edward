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


static bool verbose = false;

void add_class(ParserKaba *p, const kaba::Class *c, const string &ns);

bool allowed(const string &s) {
	if (s == "filename" or s == "config")
		return false;
	return true;
}

void add_class_content(ParserKaba *p, const kaba::Class *c, const string &ns) {
	for (auto v: c->static_variables)
		if (allowed(ns + v->name))
			p->global_variables.add(ns + v->name);
	for (auto cc: c->constants)
		p->constants.add(ns + cc->name);
	for (auto f: c->functions)
		p->functions.add(ns + f->name);
	for (auto cc: weak(c->classes))
		add_class(p, cc, ns);
}

void add_scope_content(ParserKaba *p, kaba::Scope& scope, const string &ns) {
	for (auto& e: scope.entries) {
		if (e.kind == kaba::NodeKind::Class)
			add_class(p, reinterpret_cast<const kaba::Class*>(e.p), ns);
		if (e.kind == kaba::NodeKind::Constant)
			p->constants.add(ns + reinterpret_cast<const kaba::Constant*>(e.p)->name);
		if (e.kind == kaba::NodeKind::Function)
			p->functions.add(ns + reinterpret_cast<const kaba::Function*>(e.p)->name);
		if (e.kind == kaba::NodeKind::VarGlobal) {
			auto v = reinterpret_cast<const kaba::Variable *>(e.p);
			if (allowed(ns + v->name))
				p->global_variables.add(ns + v->name);
		}
	}
}

void add_class(ParserKaba *p, const kaba::Class *c, const string &ns) {
	p->types.add(ns + c->name);
	add_class_content(p, c, ns + c->name + ".");
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

	module = nullptr;
	context = kaba::default_context->create_new_context();

	try {
		//msg_write(kaba::config.directory.str());
		module = context->create_module_for_source(text, filename, true);

		clear_symbols();

		add_scope_content(this, module->tree->global_scope, "");

		//m->tree->

		/*for (auto c: weak(m->tree->imported_symbols->classes)) {
			if (c->name.tail(1) == "*" or c->name.tail(2) == "[]")
				continue;
			add_class(this, c, "");
			add_class_content(this, c, "");
		}*/

//		add_class_content(this, m->tree->imported_symbols.get(), "");
		add_class_content(this, module->tree->base_class, "");

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
	} catch (Exception &e) {
		errors.add({"", e.message(), 0});
		//msg_error(e.message());
	}

	clear_symbols();
	for (auto p: weak(context.get()->internal_packages)) {
		add_class(this, p->main_module->base_class(), "");
		//if (p->used_by_default)
			add_class_content(this, p->main_module->base_class(), "");
	}
}


Array<Parser::Label> ParserKaba::find_labels(const string& text) {
	Array<Label> labels;

	auto ff = [] (const string& s) {
		auto x = s.replace(" virtual ", " ").replace(" extern ", " ").replace(" mut ", " ").replace(" selfref ", " ").replace(" globalref ", " ").replace(" pure ", " ").replace(" override ", " ").replace(" static ", " ").replace(" as shared", "");
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
		if (ll.head(5) == "class" or ll.head(6) == "struct" or ll.head(4) == "enum" or ll.head(5) == "trait" or ll.head(4) == "func") {
			labels.add({ff(ll), line_no, level});
		}
	}
	return labels;
}

Array<Parser::Error> ParserKaba::find_errors(const string &text) {
	return errors;
}

Array<Markup> ParserKaba::create_markup(const string &text, int offset) {
	auto markup = create_markup_default(text, offset);
	for (auto& m: markup) {

		string word_namespace;
		string temp;
		if (m.end - m.start < 64)
			temp = text.sub_ref(m.start, m.end - m.start);
		if (m.start == 0)
			word_namespace = "";

		if (m.type == MarkupType::WORD) {
			//word_namespace
			//if ((start == 0) or (p0[-1] != '.')) {
			//auto type2 = word_type(word_namespace + temp);
			auto type2 = word_type(temp);
			//	if (type2 != NONE)
			m.type = type2;
			if (m.type == MarkupType::TYPE)
				word_namespace += temp;
			else
				word_namespace = "";
			//}
		} else if (temp == ".") {
			word_namespace += ".";
		} else {
			word_namespace = "";
		}
	}
	return markup;
}


#if 0
void ParserKaba::CreateTextColors(SourceView *sv, int first_line, int last_line) {

	/*try {
		auto m = kaba::create_for_source(sv->get_all());
		msg_write("ok");
	} catch (Exception &e) {
		msg_error(e.message());
	}*/



	CreateTextColorsDefault(sv, first_line, last_line);
}
#endif




namespace kaba {


const kaba::Class *simplify_type(const kaba::Class *c) {
	if (c->is_reference())
		return c->param[0];
	return c;
}

const kaba::Class *node_namespace(shared<kaba::Node> n) {
	if (n->kind == NodeKind::Class)
		return n->as_class();
	return simplify_type(n->type);
}


bool allow(const string &name) {
	if (name.head(1) == "-")
		return false;
	if (name.head(1) == "@")
		return false;
	if (name.find(".") >= 0)
		return false;
	if (name.find("[") >= 0 or name.find("*") >= 0 or name.find("{") >= 0 or name.find("&") >= 0)
		return false;
	return true;
}

autocomplete::Data find_top_level_from_class(const Class *t, const string &yyy) {
	t = simplify_type(t);
	autocomplete::Data suggestions;
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

autocomplete::Data find_top_level(SyntaxTree *syntax, Function *f, const string &yyy) {
	autocomplete::Data suggestions;

	// general expressions
	Array<string> expressions = {"class", "func", "extends", "while", "virtual", "extern", "override", "enum", "and", "or", "while", "for", "if", "else", "const", "selfref", "new", "delete", "break", "continue", "return", "pass" "use", "import", "in", "is"};
	for (string &e: expressions)
		if (yyy == e.head(yyy.num))
			suggestions.add(e, e);

	// function local
	if (f) {
		for (auto v: f->var)
			if (yyy == v->name.head(yyy.num) and allow(v->name))
				suggestions.add(v->name, v->type->name + " " + v->name);
		if (f->name_space)
			suggestions.append(find_top_level_from_class(f->name_space, yyy));
	}

	// global var
	for (auto v: syntax->root_of_all_evil->var)
		if (yyy == v->name.head(yyy.num) and allow(v->name))
			suggestions.add(v->name, v->type->name + " " + v->name);
	if (f) {
		for (auto f: syntax->base_class->functions)
			if (yyy == f->name.head(yyy.num) and allow(f->name))
				suggestions.add(f->name, f->signature());
	}


	suggestions.append(find_top_level_from_class(syntax->base_class, yyy));

	for (auto i: syntax->includes)
		suggestions.append(find_top_level_from_class(i->tree->base_class, yyy));
	return suggestions;
}


// get deep "tail" block
Block* guess_block(SyntaxTree *syntax, Function *f) {
	Block *b = f->block;
	/*while (true){
		Block *b_next = nullptr;
		for (auto *n: b->nodes){
			if (n->kind == NodeKind::BLOCK)
				b_next = n->as_block();
		}
		if (b_next)
			b = b_next;
		else
			break;
	}*/
	return b;
	//syntax->blocks.back()
}

autocomplete::Data simple_parse(SyntaxTree *syntax, Function *f, const string &cur_line) {
	autocomplete::Data data;
	Array<string> ops = {"+", "-", "*", "/", "=", "/=", "*=", "+=", "&", "%", "and", "or", "!", "(", ")", ",", "\t"};
	string xx = cur_line;
	for (string &o: ops)
		xx = xx.replace(o, " ");
	xx = xx.explode(" ").back();
	if (verbose)
		printf("-->>>>>  %s\n", xx.c_str());
	if (xx.num == 0)
		return data;
	auto yy = xx.explode(".");
	if (verbose)
		printf("yy=%s\n", str(yy).c_str());
	data.offset = yy.back().num;
	if (yy.num == 1) {
		data.append(find_top_level(syntax, f, yy[0]));
	} else {
		if (!f)
			f = syntax->root_of_all_evil.get();

		if (verbose)
			printf("first:  %s\n", yy[0].c_str());
	//	if (syntax->blocks.num == 0)
	//		return data;
		// FIXME?

		// base layer
		Array<const Class*> types;
		auto nodes = syntax->get_existence(yy[0], guess_block(syntax, f), syntax->base_class, -1);
		if (verbose)
			printf("res: %d\n", nodes.num);
		for (auto n: nodes) {
			if (verbose)
				printf("%s\n", n->type->name.c_str());
			types.add(node_namespace(n));
		}

		// middle layers
		for (int i=1; i<yy.num-1; i++) {
			Array<const Class*> types2;
			for (auto *t: types){
				for (auto &e: t->elements)
					if (e.name == yy[i])
						types2.add(simplify_type(e.type));
				for (auto f: t->functions)
					if (f->name == yy[i])
						types2.add(simplify_type(f->literal_return_type));
				for (auto c: t->constants)
					if (c->name == yy[i])
						types2.add(simplify_type(c->type.get()));
				for (auto c: weak(t->classes))
					if (c->name == yy[i])
						types2.add(simplify_type(c));
			}
			types = types2;
		}

		// top layer
		string yyy = yy.back();
		for (auto *t: types) {
			if (verbose)
				printf("type %s\n", t->name.c_str());
			data.append(find_top_level_from_class(t, yyy));
		}
	}

	return data;
}

}



autocomplete::Data ParserKaba::run_autocomplete(const string &_code, const Path &filename, int line, int pos) {
	auto context = ownify(kaba::Context::create());
	auto s = context->create_empty_module("<auto-complete>");
	s->filename = filename;
	auto ll = _code.explode("\n");
	auto lines_pre = ll.sub_ref(0, line);//+1);
	auto lines_post = ll.sub_ref(line+1);

	string cur_line = ll[line].sub(0, pos);
	//ll2.back() = ll2.back().sub(0, pos);
	string code = implode(lines_pre, "\n") + "\n" + implode(lines_post, "\n");
	//printf("%s\n", code.c_str());
	//printf("---->>  %s\n", cur_line.c_str());
	s->just_analyse = true;
	autocomplete::Data data;
	kaba::Function *ff = nullptr;
	s->tree->parser = new kaba::Parser(s->tree.get());

	try {
		//s->tree->ParseBuffer(code, true);

		s->tree->default_import();

		//printf("--a\n");
		s->tree->parser->Exp.analyse(s->tree.get(), code);

		//printf("--b\n");
		s->tree->parser->parse_legacy_macros(true);

		//printf("--c\n");
		s->tree->parser->parse_abstract_top_level();



	} catch (const kaba::Exception &e) {
		printf("err: %s\n", e.message().c_str());
		//if (e.line)
		//throw e;
		//data = simple_parse(s->tree.get(), ff, cur_line);
	}





	//printf("--d\n");
	for (auto *f: s->tree->functions) {
		int f_line_no = s->tree->parser->Exp.token_logical_line(f->token_id)->physical_line;
		if (!f->is_extern() and (f_line_no >= 0) and (f_line_no < line))
			ff = f;
	}

	data = simple_parse(s->tree.get(), ff, cur_line);




	for (int i=0; i<data.suggestions.num; i++)
		for (int j=i+1; j<data.suggestions.num; j++) {
			if (data.suggestions[j].context == data.suggestions[i].context) {
				data.suggestions.erase(j);
				j --;
			} else if (data.suggestions[j].name < data.suggestions[i].name) {
				data.suggestions.swap(i, j);
			}
		}
	return data;
}

base::optional<Parser::SymbolInfo> node_info(kaba::Node* n) {
	auto xxx = [] (kaba::Module* m, int token_id, const string& description) -> Parser::SymbolInfo {
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

	n->show();
	if (n->kind == kaba::NodeKind::Class) {
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
		return xxx(v->ns->owner->module, v->token_id, "variable  " + v->type->long_name());
	} else if (n->kind == kaba::NodeKind::Statement) {
		return xxx(nullptr, -1, "statement");
	}
	return base::None;
}

base::optional<Parser::SymbolInfo> ParserKaba::symbol_info(const string& text, int offset, int length) {
	if (!module)
		return base::None;

	// TODO search node
	string x = text.sub(offset, offset + length);
	auto xx = module->tree->get_existence(x, module->tree->root_of_all_evil->block, module->tree->base_class, -1);

	for (auto n: weak(xx))
		return node_info(n);
	return base::None;
}
