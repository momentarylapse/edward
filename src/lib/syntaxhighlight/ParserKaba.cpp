/*
 * ParserKaba.cpp
 *
 *  Created on: 06.06.2013
 *      Author: michi
 */

#include "ParserKaba.h"
//#ifdef SYNTAX_HIGHLIGHT_KABA ...
#include <lib/kaba/kaba.h>

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
	return create_markup_default(text, offset);
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
