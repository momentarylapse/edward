/*
 * ParserCmake.cpp
 *
 *  Created on: 25.06.2013
 *      Author: michi
 */

#include "ParserCmake.h"

ParserCmake::ParserCmake() : Parser("Python") {
	line_comment_begin = "#";
	string_sub_begin = "${";
	string_sub_end = "}";
	keywords.add("if");
	keywords.add("else");
	keywords.add("endif");
	keywords.add("macro");
	keywords.add("endmacro");

	//operator_functions
	keywords.add("APPEND");
	keywords.add("AND");
	keywords.add("FILTER");
	keywords.add("FIND");
	keywords.add("GET");
	keywords.add("IN_LIST");
	keywords.add("JOIN");
	keywords.add("LENGTH");
	keywords.add("MATCH");
	keywords.add("NOT");
	keywords.add("OR");
	keywords.add("REVERSE");
	keywords.add("SORT");
	keywords.add("STREQUAL");
	keywords.add("SUBLIST");

	types.add("BOOL");
	types.add("PATH");
	types.add("FILEPATH");
	types.add("STRING");
	types.add("UNINITIALIZED");

	//modifiers
	modifiers.add("CACHE");
	types.add("CODE");
	types.add("COMMAND");
	types.add("DEPENDS");
	types.add("DESTINATION");
	types.add("DIRECTORIES");
	types.add("DIRECTORY");
	types.add("FILES");
	modifiers.add("FORCE");
	types.add("INSTALL_PREFIX");
	modifiers.add("INTERFACE");
	modifiers.add("NO_SOURCE_PERMISSIONS");
	types.add("ON");
	types.add("OUTPUT");
	types.add("OFF");
	types.add("PROPERTIES");
	modifiers.add("PUBLIC");
	modifiers.add("PRIVATE");
	modifiers.add("REQUIRED");
	types.add("SEND_ERROR");
	types.add("STATUS");
	types.add("TARGETS");
	types.add("VERSION");
	types.add("WARNING");
	global_variables.add("APPLE");
	global_variables.add("CMAKE_CURRENT_SOURCE_DIR");
	global_variables.add("CMAKE_CXX_STANDARD");
	global_variables.add("CMAKE_CXX_STANDARD_REQUIRED");
	global_variables.add("CMAKE_BUILD_TYPE");
	global_variables.add("CMAKE_SYSTEM");
	global_variables.add("CMAKE_SYSTEM_VERSION");
	global_variables.add("MSVC");
	global_variables.add("MACOSX_BUNDLE");
	global_variables.add("MACOSX_BUNDLE_ICON_FILE");
	global_variables.add("MACOSX_BUNDLE_BUNDLE_NAME");
	global_variables.add("MACOSX_BUNDLE_GUI_IDENTIFIER");
	global_variables.add("MACOSX_BUNDLE_BUNDLE_VERSION");
	global_variables.add("MACOSX_BUNDLE_SHORT_VERSION_STRING");
	global_variables.add("MACOSX_PACKAGE_LOCATION");
	global_variables.add("THREADS_PREFER_PTHREAD_FLAG");
	global_variables.add("UNIX");
	global_variables.add("WIN32");
	compiler_functions.add("add_custom_command");
	compiler_functions.add("add_executable");
	compiler_functions.add("add_library");
	compiler_functions.add("cmake_minimum_required");
	compiler_functions.add("configure_file");
	compiler_functions.add("copy_if_different");
	compiler_functions.add("execute_process");
	compiler_functions.add("file");
	compiler_functions.add("find_library");
	compiler_functions.add("find_package");
	compiler_functions.add("find_program");
	compiler_functions.add("include");
	compiler_functions.add("install");
	compiler_functions.add("list");
	compiler_functions.add("message");
	compiler_functions.add("pkg_check_modules");
	compiler_functions.add("project");
	compiler_functions.add("set");
	compiler_functions.add("set_source_files_properties");
	compiler_functions.add("set_target_properties");
	compiler_functions.add("target_compile_options");
	compiler_functions.add("target_compile_definitions");
	compiler_functions.add("target_include_directories");
	compiler_functions.add("target_link_directories");
	compiler_functions.add("target_link_libraries");

	constants.add("True");
	constants.add("true");
	constants.add("False");
	constants.add("false");
}

Array<Markup> ParserCmake::create_markup(const string &text, int offset) {
	return create_markup_default(text, offset);
}

