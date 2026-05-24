#include <lib/codeeditor/CodeEditor.h>
#include <lib/kapi/KabaExporter.h>


codeedit::CodeEditor* create_code_editor() {
	return new codeedit::CodeEditor();
}

void _export_package_codeeditor_internal(kaba::IExporter* e) {
	e->declare_class_size("CodeEditor", sizeof(codeedit::CodeEditor));
	e->declare_class_element("CodeEditor.out_changed", &codeedit::CodeEditor::out_changed);
	e->declare_class_element("CodeEditor.out_info", &codeedit::CodeEditor::out_info);
	e->declare_class_element("CodeEditor.out_error", &codeedit::CodeEditor::out_error);
	e->declare_class_element("CodeEditor.out_request_open_file", &codeedit::CodeEditor::out_request_open_file);
	e->declare_class_element("CodeEditor.filename", &codeedit::CodeEditor::filename);
	e->declare_class_element("CodeEditor.id_edit", &codeedit::CodeEditor::id_edit);
	e->declare_class_element("CodeEditor.id_structure", &codeedit::CodeEditor::id_structure);
	e->link_class_func("CodeEditor.load", &codeedit::CodeEditor::load);
	e->link_class_func("CodeEditor.save", &codeedit::CodeEditor::save);
	e->link_class_func("CodeEditor.title", &codeedit::CodeEditor::title);
	e->link_class_func("CodeEditor.undo", &codeedit::CodeEditor::undo);
	e->link_class_func("CodeEditor.redo", &codeedit::CodeEditor::redo);
	e->link_class_func("CodeEditor.is_undoable", &codeedit::CodeEditor::is_undoable);
	e->link_class_func("CodeEditor.is_redoable", &codeedit::CodeEditor::is_redoable);
	e->link_class_func("CodeEditor.is_save_state", &codeedit::CodeEditor::is_save_state);
	e->link_class_func("CodeEditor.set_save_state", &codeedit::CodeEditor::set_save_state);
	e->link_class_func("CodeEditor.set_cursor_pos", &codeedit::CodeEditor::set_cursor_pos);

	e->link_func("create_code_editor", &create_code_editor);
}

void export_package_codeeditor(kaba::IExporter* e) {
	e->package_info("codeeditor", "0.1");
	_export_package_codeeditor_internal(e);
}
