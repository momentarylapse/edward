//
// Created by michi on 9/9/25.
//

#pragma once

#include <lib/xhui/Window.h>
#include <lib/pattern/Observable.h>

namespace codeedit {

class DocumentEditor;
class DocumentSwitcher;

class EditorWindow : public obs::Node<xhui::Window> {
public:
	EditorWindow();

	void on_key_up(int key) override;

	DocumentEditor* create_document_editor();

	owned_array<DocumentEditor> document_editors;
	DocumentEditor* active_editor = nullptr;
	void set_active(DocumentEditor* editor);

	void update_title();

	DocumentSwitcher* switcher = nullptr;
};

}
