#include "../kaba.h"
#include "lib.h"
#include "../dynamic/exception.h"
#include "../dynamic/dynamic.h"


namespace kaba {

extern const Class *TypePath;

#pragma GCC push_options
#pragma GCC optimize("no-omit-frame-pointer")
#pragma GCC optimize("no-inline")
#pragma GCC optimize("0")

shared<Script> __load_script__(const string &filename, bool just_analyse) {
	KABA_EXCEPTION_WRAPPER( return load(filename, just_analyse); );
	return nullptr;
}

shared<Script> __create_from_source__(const string &source, bool just_analyse) {
	KABA_EXCEPTION_WRAPPER( return create_for_source(source, just_analyse); );
	return nullptr;
}

void __execute_single_command__(const string &cmd) {
	KABA_EXCEPTION_WRAPPER( execute_single_script_command(cmd); );
}

#pragma GCC pop_options


void show_func(Function *f) {
	config.verbose = true;
	f->show("");
	config.verbose = false;
}

class ClassX : public Class {
public:
	string repr() const {
		return class_repr(this);
	}
};

class FunctionX : public Function {
public:
	string repr() const {
		return func_repr(this);
	}
};

void SIAddPackageKaba() {
	add_package("kaba");


	TypeClass 			= add_type  ("Class", sizeof(Class));
	TypeClassP			= add_type_p(TypeClass);
	auto *TypeClassPList = add_type_l(TypeClassP);

	TypeFunction		= add_type  ("Function", sizeof(Function));
	TypeFunctionP		= add_type_p(TypeFunction);
	auto *TypeFunctionPList = add_type_l(TypeFunctionP);
	TypeFunctionCode	= add_type  ("code", 32); // whatever
	TypeFunctionCodeP	= add_type_p(TypeFunctionCode);
	auto *TypeStatement = add_type  ("Statement", sizeof(Statement));
	auto *TypeStatementP= add_type_p(TypeStatement);
	auto *TypeStatementPList = add_type_l(TypeStatementP);
		

	auto *TypeScript = add_type  ("Script", sizeof(Script));
	auto *TypeScriptP = add_type_p(TypeScript, Flags::SHARED);
	auto *TypeScriptPList = add_type_l(TypeScriptP);

	
	auto *TypeClassElement = add_type("ClassElement", sizeof(ClassElement));
	auto *TypeClassElementList = add_type_l(TypeClassElement);
	auto *TypeVariable = add_type("Variable", sizeof(Variable));
	auto *TypeVariableP = add_type_p(TypeVariable);
	auto *TypeVariablePList = add_type_l(TypeVariableP);
	auto *TypeConstant = add_type("Constant", sizeof(Constant));
	auto *TypeConstantP = add_type_p(TypeConstant);
	auto *TypeConstantPList = add_type_l(TypeConstantP);
	
	
	add_class(TypeClassElement);
		class_add_element("name", TypeString, &ClassElement::name);
		class_add_element("type", TypeClassP, &ClassElement::type);
		class_add_element("offset", TypeInt, &ClassElement::offset);


	add_class(TypeClass);
		class_add_element("name", TypeString, &Class::name);
		class_add_element("size", TypeInt, &Class::size);
		class_add_element("type", TypeInt, &Class::type);
		class_add_element("parent", TypeClassP, &Class::parent);
		class_add_element("param", TypeClassPList, &Class::param);
		class_add_element("namespace", TypeClassP, &Class::name_space);
		class_add_element("elements", TypeClassElementList, &Class::elements);
		class_add_element("functions", TypeFunctionPList, &Class::functions);
		class_add_element("classes", TypeClassPList, &Class::classes);
		class_add_element("constants", TypeConstantPList, &Class::constants);
		class_add_element("static_variables", TypeVariablePList, &Class::static_variables);
		class_add_func("is_derived_from", TypeBool, &Class::is_derived_from, Flags::PURE);
			func_add_param("c", TypeClass);
		class_add_func("is_pointer", TypeBool, &Class::is_pointer, Flags::PURE);
		class_add_func("is_super_array", TypeBool, &Class::is_super_array, Flags::PURE);
		class_add_func("is_array", TypeBool, &Class::is_array, Flags::PURE);
		class_add_func("is_dict", TypeBool, &Class::is_dict, Flags::PURE);
		class_add_func("get_func", TypeFunctionP, &Class::get_func, Flags::PURE); // selfref
			func_add_param("name", TypeString);
			func_add_param("return_type", TypeClass);
			func_add_param("params", TypeClassPList);
		class_add_func("long_name", TypeString, &Class::long_name, Flags::PURE);
		class_add_func("cname", TypeString, &Class::cname, Flags::PURE);
			func_add_param("ns", TypeClassP);
		class_add_func(IDENTIFIER_FUNC_STR, TypeString, &ClassX::repr, Flags::PURE);

	add_class(TypeClassP);
		class_add_func(IDENTIFIER_FUNC_STR, TypeString, &class_repr, Flags::PURE);

	add_class(TypeFunction);
		class_add_element("name", TypeString, &Function::name);
		class_add_func("long_name", TypeString, &Function::long_name, Flags::PURE);
		class_add_func("cname", TypeString, &Function::cname, Flags::PURE);
			func_add_param("ns", TypeClassP);
		class_add_func("signature", TypeString, &Function::signature, Flags::PURE);
			func_add_param("ns", TypeClassP);
		class_add_element("namespace", TypeClassP, &Function::name_space);
		class_add_element("num_params", TypeInt, &Function::num_params);
		class_add_element("var", TypeVariablePList, &Function::var);
		class_add_element("param_type", TypeClassPList, &Function::literal_param_type);
		class_add_element("return_type", TypeClassP, &Function::literal_return_type);
		class_add_func("is_static", TypeBool, &Function::is_static, Flags::PURE);
		class_add_func("is_pure", TypeBool, &Function::is_pure, Flags::PURE);
		class_add_func("is_const", TypeBool, &Function::is_const, Flags::PURE);
		class_add_func("is_extern", TypeBool, &Function::is_extern, Flags::PURE);
		class_add_func("is_selfref", TypeBool, &Function::is_selfref, Flags::PURE);
		class_add_func("throws_exceptions", TypeBool, &Function::throws_exceptions, Flags::PURE);
		class_add_element("needs_overriding", TypeBool, &Function::needs_overriding);
		class_add_element("virtual_index", TypeInt, &Function::virtual_index);
		class_add_element("inline_index", TypeInt, &Function::inline_no);
		class_add_element("code", TypeFunctionCodeP, &Function::address);
		class_add_func(IDENTIFIER_FUNC_STR, TypeString, &FunctionX::repr, Flags::PURE);

	//add_class(TypeFunctionP);

	add_class(TypeVariable);
		class_add_element("name", TypeString, &Variable::name);
		class_add_element("type", TypeClassP, &Variable::type);
		class_add_func("is_const", TypeBool, &Variable::is_const);
		class_add_func("is_extern", TypeBool, &Variable::is_extern);
		
	add_class(TypeConstant);
		class_add_element("name", TypeString, &Constant::name);
		class_add_element("type", TypeClassP, &Constant::type);

	add_class(TypeScript);
		class_add_element("name", TypeString, &Script::filename);
		class_add_element("used_by_default", TypeBool, &Script::used_by_default);
		class_add_func("classes", TypeClassPList, &Script::classes, Flags::PURE);
		class_add_func("functions", TypeFunctionPList, &Script::functions, Flags::PURE);
		class_add_func("variables", TypeVariablePList, &Script::variables, Flags::PURE);
		class_add_func("constants", TypeConstantPList, &Script::constants, Flags::PURE);
		class_add_func("base_class", TypeClassP, &Script::base_class, Flags::PURE);
		class_add_func("load", TypeScriptP, &__load_script__, Flags::_STATIC__RAISES_EXCEPTIONS);
			func_add_param("filename", TypePath);
			func_add_param("just_analize", TypeBool);
		class_add_func("create", TypeScriptP, &__create_from_source__, Flags::_STATIC__RAISES_EXCEPTIONS);
			func_add_param("source", TypeString);
			func_add_param("just_analize", TypeBool);
		class_add_func("delete", TypeVoid, &remove_script, Flags::STATIC);
			func_add_param("script", TypeScript);
		class_add_func("execute_single_command", TypeVoid, &__execute_single_command__, Flags::_STATIC__RAISES_EXCEPTIONS);
			func_add_param("cmd", TypeString);
	
	add_class(TypeStatement);
		class_add_element("name", TypeString, &Statement::name);
		class_add_element("id", TypeInt, &Statement::id);
		class_add_element("num_params", TypeInt, &Statement::num_params);
		
	add_class(TypeScriptPList);
		class_add_func(IDENTIFIER_FUNC_INIT, TypeVoid, &shared_array<Script>::__init__);

	add_class(TypeClassElementList);
		class_add_func(IDENTIFIER_FUNC_INIT, TypeVoid, &Array<ClassElement>::__init__);
		class_add_func(IDENTIFIER_FUNC_DELETE, TypeVoid, &Array<ClassElement>::clear);

	add_func("get_dynamic_type", TypeClassP, &get_dynamic_type, Flags::_STATIC__PURE);
		func_add_param("p", TypePointer);
	add_func("disassemble", TypeString, &Asm::disassemble, Flags::_STATIC__PURE);
		func_add_param("p", TypePointer);
		func_add_param("length", TypeInt);
		func_add_param("comments", TypeBool);
	add_func("show_func", TypeVoid, &show_func, Flags::STATIC);
		func_add_param("f", TypeFunction);

	add_ext_var("packages", TypeScriptPList, (void*)&packages);
	add_ext_var("statements", TypeStatementPList, (void*)&Statements);
	add_ext_var("kaba_version", TypeString, (void*)&Version);
}



}
