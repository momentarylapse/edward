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
#include <stdio.h>

static bool verbose = false;

#include "lib/os/msg.h"

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
	keywords.add(kaba::Identifier::Enum);
	keywords.add(kaba::Identifier::Class);
	keywords.add(kaba::Identifier::Struct);
	keywords.add(kaba::Identifier::Interface);
	keywords.add(kaba::Identifier::Func);
	keywords.add(kaba::Identifier::Macro);
	keywords.add(kaba::Identifier::Extends);
	keywords.add(kaba::Identifier::Use);
	keywords.add(kaba::Identifier::Asm);
	keywords.add(kaba::Identifier::Import);
	keywords.add(kaba::Identifier::If);
	keywords.add(kaba::Identifier::Else);
	keywords.add(kaba::Identifier::While);
	keywords.add(kaba::Identifier::For);
	keywords.add(kaba::Identifier::In);
	keywords.add(kaba::Identifier::Match);
	keywords.add(kaba::Identifier::Return);
	keywords.add(kaba::Identifier::Break);
	keywords.add(kaba::Identifier::Continue);
	keywords.add(kaba::Identifier::Not);
	keywords.add(kaba::Identifier::And);
	keywords.add(kaba::Identifier::Or);
	keywords.add(kaba::Identifier::New);
	keywords.add(kaba::Identifier::Delete);
	modifiers.add(kaba::Identifier::Extern);
	modifiers.add(kaba::Identifier::Virtual);
	modifiers.add(kaba::Identifier::Override);
	modifiers.add(kaba::Identifier::Static);
	keywords.add(kaba::Identifier::Const);
	modifiers.add(kaba::Identifier::Mutable);
	modifiers.add(kaba::Identifier::Selfref);
	modifiers.add(kaba::Identifier::Ref);
	modifiers.add(kaba::Identifier::Out);
	modifiers.add(kaba::Identifier::Shared);
	modifiers.add(kaba::Identifier::Owned);
	modifiers.add(kaba::Identifier::Xfer);
	modifiers.add(kaba::Identifier::RawPointer);
	modifiers.add(kaba::Identifier::Future);
	keywords.add(kaba::Identifier::Self);
	keywords.add(kaba::Identifier::Super);
	keywords.add(kaba::Identifier::Namespace);
	keywords.add(kaba::Identifier::Raise);
	keywords.add(kaba::Identifier::Try);
	keywords.add(kaba::Identifier::Except);
	keywords.add(kaba::Identifier::Pass);
	keywords.add(kaba::Identifier::Let);
	keywords.add(kaba::Identifier::Var);
	keywords.add(kaba::Identifier::Lambda);
	keywords.add(kaba::Identifier::RawFunctionPointer);
	keywords.add(kaba::Identifier::TrustMe);
	compiler_functions.add(kaba::Identifier::Dyn);
	compiler_functions.add(kaba::Identifier::Weak);
	compiler_functions.add(kaba::Identifier::Give);
	compiler_functions.add(kaba::Identifier::Len);
	compiler_functions.add(kaba::Identifier::Sizeof);
	compiler_functions.add(kaba::Identifier::Str);
	compiler_functions.add(kaba::Identifier::Typeof);
	compiler_functions.add(kaba::Identifier::Sort);
	compiler_functions.add(kaba::Identifier::Filter);
	operator_functions.add(kaba::Identifier::func::Init);
	operator_functions.add(kaba::Identifier::func::Delete);
	operator_functions.add(kaba::Identifier::func::DeleteOverride);
	operator_functions.add(kaba::Identifier::func::Assign);
	operator_functions.add(kaba::Identifier::func::Contains);
	operator_functions.add(kaba::Identifier::func::Str);
	operator_functions.add(kaba::Identifier::func::Repr);
	operator_functions.add(kaba::Identifier::func::Set);
	operator_functions.add(kaba::Identifier::func::Get);
	operator_functions.add(kaba::Identifier::func::Subarray);
	operator_functions.add(kaba::Identifier::func::Add);
	operator_functions.add(kaba::Identifier::func::AddAssign);
	operator_functions.add(kaba::Identifier::func::Subtract);
	operator_functions.add(kaba::Identifier::func::SubtractAssign);
	operator_functions.add(kaba::Identifier::func::Multiply);
	operator_functions.add(kaba::Identifier::func::MultiplyAssign);
	operator_functions.add(kaba::Identifier::func::Divide);
	operator_functions.add(kaba::Identifier::func::DivideAssign);
	operator_functions.add(kaba::Identifier::func::Smaller);
	operator_functions.add(kaba::Identifier::func::SmallerEqual);
	operator_functions.add(kaba::Identifier::func::Greater);
	operator_functions.add(kaba::Identifier::func::GreaterEqual);
	operator_functions.add(kaba::Identifier::func::Equal);
	operator_functions.add(kaba::Identifier::func::NotEqual);
	operator_functions.add(kaba::Identifier::func::Modulo);
	operator_functions.add(kaba::Identifier::func::Increase);
	operator_functions.add(kaba::Identifier::func::Decrease);
	operator_functions.add(kaba::Identifier::func::Exponent);
	operator_functions.add(kaba::Identifier::func::ShiftLeft);
	operator_functions.add(kaba::Identifier::func::ShiftRight);
	operator_functions.add(kaba::Identifier::func::And);
	operator_functions.add(kaba::Identifier::func::Or);
	operator_functions.add(kaba::Identifier::func::Not);
	operator_functions.add(kaba::Identifier::func::Negative);
	operator_functions.add(kaba::Identifier::func::BitAnd);
	operator_functions.add(kaba::Identifier::func::BitOr);
	operator_functions.add(kaba::Identifier::func::MapsTo);
	operator_functions.add(kaba::Identifier::func::Call);
	keywords.add("as");
	//for (auto &s: kaba::Statements)
	//	special_words.add(s.name);
}

void ParserKaba::clear_symbols() {
	types.clear();
	global_variables.clear();
	functions.clear();
	constants.clear();
}


void ParserKaba::prepare_symbols(const string &text, const Path& filename) {

	context = kaba::Context::create();

	try {
		kaba::config.default_filename = filename;
		//msg_write(kaba::config.directory.str());
		auto m = context->create_module_for_source(text, true);

		clear_symbols();

		add_scope_content(this, m->tree->global_scope, "");

		//m->tree->

		/*for (auto c: weak(m->tree->imported_symbols->classes)) {
			if (c->name.tail(1) == "*" or c->name.tail(2) == "[]")
				continue;
			add_class(this, c, "");
			add_class_content(this, c, "");
		}*/

//		add_class_content(this, m->tree->imported_symbols.get(), "");
		add_class_content(this, m->tree->base_class, "");

	} catch (Exception &e) {
		//msg_error(e.message());
	}

	for (auto p: weak(context->internal_packages)) {
		add_class(this, p->base_class(), "");
		//if (p->used_by_default)
			add_class_content(this, p->base_class(), "");
	}
}


Array<Parser::Label> ParserKaba::find_labels(const string &text, int offset) {
	Array<Parser::Label> labels;

#if 0
	int num_lines = sv->get_num_lines();
	string last_class;
	for (int l=0;l<num_lines;l++) {
		string s = sv->get_line(l);
		if (s.num < 4)
			continue;
		if (char_type(s[0]) == CHAR_LETTER) {
			if (s.find("class ") >= 0) {
				last_class = s.replace("\t", " ").replace(":", " ").explode(" ")[1];
				s = "class " + last_class;
			} else if (s.find("(") >= 0) {
				last_class = "";
			} else {
				continue;
			}
			if (s.find(kaba::Identifier::Extern) >= 0)
				continue;
			labels.add(Label(s, l, 0));
		} else if ((last_class.num > 0) && (s[0] == '\t') && (char_type(s[1]) == CHAR_LETTER)) {
			if (s.find("(") < 0)
				continue;
			s = s.replace(kaba::Identifier::Virtual + " ", "").replace(kaba::Identifier::Override + " ", "").trim();
			labels.add(Label(s, l, 1));
		}
	}
#endif
	return labels;
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


void _ParseFunctionBody(SyntaxTree *syntax, Function *f) {
	syntax->parser->Exp.cur_line = syntax->parser->Exp.token_logical_line(f->token_id);

	int indent0 = syntax->parser->Exp.cur_line->indent;
	bool more_to_parse = true;

	syntax->parser->parser_loop_depth = 0;

// instructions
	try {
		while (more_to_parse) {
			more_to_parse = syntax->parser->parse_abstract_indented_command_into_block(f->block.get(), indent0);
		}
	} catch (...) {}
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
	Block *b = f->block.get();
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
		s->tree->parser->parse_top_level();



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
	if (ff) {
//		printf("func: %s\n", ff->name.c_str());
		_ParseFunctionBody(s->tree.get(), ff);
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
