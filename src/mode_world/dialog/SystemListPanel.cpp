//
// Created by michi on 7/6/26.
//

#include "SystemListPanel.h"
#include "components/ComponentSelectionDialog.h"
#include <mode_world/data/DataWorld.h>
#include <lib/os/msg.h>
#include <lib/xhui/xhui.h>
#include <lib/base/iter.h>
#include <lib/xhui/Menu.h>
#include <lib/xhui/controls/ListView.h>
#include <mode_world/action/ActionWorldEditData.h>
#include <storage/Storage.h>
#include <view/dialogs/CommonDialogs.h>
#include <world/systems/Physics.h>
#include <stuff/PluginManager.h>


class SystemPanel : public obs::Node<xhui::Panel> {
public:
	explicit SystemPanel(SystemListPanel* _list_panel) : obs::Node<xhui::Panel>("") {
		properties_dialog = _list_panel;
		data = properties_dialog->data;
		from_source(R"foodelim(
Dialog system 'System'
	Grid ? '' class=card
		Grid ? '' left
			Label class '' bold
			Label filename '' small
		---|
		Expander contents '' expandx
			Grid ? ''
				Grid variables ''
				---|
				Separator ? "" horizontal
				---|
				Grid ? ''
					Label ? '' expandx
					Button edit 'Edit code' tooltip='Edit script file that defines this system class' primary noexpandx
					Button delete 'Delete' tooltip='Remove system' danger noexpandx
)foodelim");
		event("delete", [this] {
			if (index >= 0) {
				properties_dialog->temp = data->meta_data;
				properties_dialog->temp.systems.erase(index);
				properties_dialog->apply();
			}
		});
		event("edit", [this] {
			data->session->universal_edit(FD_SCRIPT, data->meta_data.systems[index].filename, true);
		});
		data->out_system_changed >> create_data_sink<int>([this] (int _index) {
			if (_index == index and !editing)
				update_ui();
		});
	}
	void set_index(int _index) {
		if (editing)
			return;
		index = _index;
		auto& s = data->meta_data.systems[index];
		set_target("variables");
		for (const auto& [i, v]: enumerate(s.variables)) {
			string id_var = format("var-%d", i);
			add_control("Label", v.name, 0, i, format("l-var-%d", i));
			set_options(format("l-var-%d", i), "right,disabled");
			add_control("Edit", str(v.value), 1, i, id_var);
			event(id_var, [this, i=i, id=id_var] {
				properties_dialog->temp = data->meta_data;
				properties_dialog->temp.systems[index].variables[i].value = Any::parse(get_string(id));
				editing = true;
				properties_dialog->apply();
				editing = false;
			});
		}
		if (s.variables.num == 0)
			add_control("Label", "no variables", 0, 0, "");
		update_ui();
	}
	void update_ui() {
		auto& s = data->meta_data.systems[index];
		set_string("class", s.class_name);
		set_string("filename", str(s.filename));
		for (const auto& [i, v]: enumerate(s.variables)) {
			string id_var = format("var-%d", i);
			set_string(id_var, str(v.value));
		}
	}
	void set_selected(bool selected) {
		expand("contents", selected);
	}
	SystemListPanel* properties_dialog;
	DataWorld* data;
	int index = -1;
	bool editing = false;
};

SystemListPanel::SystemListPanel(DataWorld* _data) : Node<xhui::Panel>("") {//: Dialog("world_dialog", parent) {
	data = _data;
	temp = data->meta_data;

	from_source(R"foodelim(
Dialog systems-panel ''
	Grid ? '' vertical padding=7
		ListView systems 'systems' nobar sunkenbackground=no showselection=no selectsingle spacing=8 tooltip='Global scripts - a class derived from yengine.System is instantiated and running in the background\n * click to edit variables etc.'
		Grid ? ''
			Button add-system '' image=open tooltip='Add system'
			Button create-system '' image=new tooltip='Create new system'
)foodelim");

	auto systems_list = (xhui::ListView*)get_control("systems");
	systems_list->column_factories[0].f_create = [this](const string& id) -> xhui::Control* {
		return new SystemPanel(this);
	};
	systems_list->column_factories[0].f_set = [](xhui::Control* c, const string& t) {
		reinterpret_cast<SystemPanel*>(c)->set_index(t._int());
	};
	systems_list->column_factories[0].f_select = [](xhui::Control* c, bool selected) {
		reinterpret_cast<SystemPanel*>(c)->set_selected(selected);
	};

	data->out_changed >> create_sink([this] {
		if (!editing) {
			temp = data->meta_data;
			update_ui();
		}
	});
	data->out_system_added >> create_sink([this] {
		temp = data->meta_data;
		fill_systems_list();
	});
	data->out_system_removed >> create_sink([this] {
		temp = data->meta_data;
		fill_systems_list();
	});

	update_ui();
	fill_systems_list();

	event_x("systems", xhui::event_id::RightButtonDown, [this] {
		auto m = new xhui::Menu;
		m->add_item("delete-system", "Delete");
		//m->add_item("system-choose", "Choose file...");
		m->add_item("add-system", "Add from file...");
		m->add_item("create-system", "Create new...");
		m->open_popup(this);
	});
	event("delete-system", [this] {
		int n = get_int("systems");
		if (n >= 0)
			temp.systems.erase(n);
		apply();
	});
	event("add-system", [this] {
		ComponentSelectionDialog::ask(this, data->session, "System").then([this] (const kaba::Class* c) {
			temp.systems.add(data->session->plugin_manager->describe_class(c));
			apply();
		});
	});
	event("create-system", [this] {
		add_new_system();
	});
}

void SystemListPanel::add_new_system() {
	data->session->storage->file_dialog(FD_SCRIPT, true, true).then([this] (const ComplexPath& path) {
		TextDialog::ask(this, "System name", "Test").then([this, path] (const string& name) {
			os::fs::write_text(path.complete, string(R"foodelim(use yengine.*

class <NAME> extends System
	var some_variable: f32

	func override on_finished_loading()
		# called when world is fully loaded


	func override on_add_component(out c: ComponentInfo)
		# called whenever a component is added to an entity in the world


	func override on_remove_component(out c: ComponentInfo)
		# called whenever a component is removed from an entity in the world


	func override on_iterate(dt: f32)
		# called in each frame


	func override on_key_down(key: Key)
		# called when a key on the keyboard is pressed down


	func override on_left_button_down()
		# called when the left mouse button is pressed down
)foodelim").replace("<NAME>", name));

			temp.systems.add({name, path.relative, {}});
			apply();
		});
	});
}


void SystemListPanel::update_ui() {
}

void SystemListPanel::fill_systems_list() {
	reset("systems");
	for (int i=0; i<temp.systems.num; i++)
		add_string("systems", str(i));
}


void SystemListPanel::apply() {
	editing = true;
	auto meta = data->meta_data;
	meta.systems = temp.systems;
	data->execute(new ActionWorldEditData(meta));
	editing = false;
}

