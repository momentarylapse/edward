/*
 * WorldObjectListPanel.h
 *
 *  Created on: Dec 28, 2020
 *      Author: michi
 */

#ifndef SRC_MODE_WORLD_DIALOG_WORLDOBJECTLISTPANEL_H_
#define SRC_MODE_WORLD_DIALOG_WORLDOBJECTLISTPANEL_H_


#include "../../../lib/hui/hui.h"
#include "../../../lib/pattern/Observable.h"

class ModeWorld;
class DataWorld;


class WorldObjectListPanel : public obs::Node<hui::Panel> {
public:
	ModeWorld *world;
	DataWorld *data;

	struct Index {
		int type, index;
	};
	Array<Index> list_indices;
	int editing;
	bool allow_sel_change_signal;
	owned<hui::Menu> popup;
	owned<hui::Menu> popup_component;

	WorldObjectListPanel(ModeWorld *w);
	~WorldObjectListPanel();
	void fill_list();
	void selection_from_world();
	void on_list_right_click();
	void on_component_list_right_click();
	void on_list_select();
	void selection_to_world(const Array<int> &sel);
	void set_editing(int s);
	void on_change();
	void on_script_edit();
	void on_component_add();
	void on_component_delete();
	void on_component_edit_variables();
	void on_object_edit();
};




#endif /* SRC_MODE_WORLD_DIALOG_WORLDOBJECTLISTPANEL_H_ */
