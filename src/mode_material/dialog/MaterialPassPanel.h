//
// Created by Michael Ankele on 2025-05-02.
//

#ifndef MATERIALPASSPANEL_H
#define MATERIALPASSPANEL_H


#include <lib/xhui/xhui.h>
#include <lib/pattern/Observable.h>

class DataMaterial;
class MaterialPanel;

class MaterialPassPanel : public xhui::Panel {
public:
	MaterialPassPanel(MaterialPanel* _parent, DataMaterial* _data, int _index);
	void update(int _index);
	void set_selected(bool selected);
	void apply_data();

	MaterialPanel* parent;
	DataMaterial* data;
	int index;
};


#endif //MATERIALPASSPANEL_H
