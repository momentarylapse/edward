//
// Created by Michael Ankele on 2025-04-21.
//

#include "EditTerrainPanel.h"
#include "../ModeEditTerrain.h"
#include "../action/terrain/ActionWorldResizeTerrain.h"
#include "../data/DataWorld.h"
#include <lib/base/tuple.h>
#include <lib/xhui/Dialog.h>
#include <lib/xhui/xhui.h>
#include <world/Terrain.h>

class IntPairDialog : public xhui::Dialog {
public:
	IntPairDialog(xhui::Panel* parent, const string& question, int a, int b) : Dialog("Question", 320, 120, parent, xhui::DialogFlags::CloseByEscape) {
		from_source(R"foodelim(
Dialog int-pair-dialog 'Question'
	Grid ? ''
		Label question ''
		---|
		Grid ? ''
			SpinButton a '' range=0:9999:1 expandx
			SpinButton b '' range=0:9999:1 expandx
		---|
		Grid ? ''
			Button ok 'Ok'
			Button cancel 'Cancel'
)foodelim");
		set_string("question", question);
		set_int("a", a);
		set_int("b", b);
		event("ok", [this] {
			promise({get_int("a"), get_int("b")});
			request_destroy();
		});
		event("cancel", [this] {
			promise.fail();
			request_destroy();
		});
		event(xhui::event_id::Close, [this] {
			promise.fail();
			request_destroy();
		});
	}
	base::promise<base::tuple<int,int>> promise;

	static base::future<base::tuple<int,int>> ask(xhui::Panel* parent, const string& question, int a, int b) {
		auto dlg = new IntPairDialog(parent, question, a, b);
		parent->open_dialog(dlg);
		return dlg->promise.get_future();
	}
};

EditTerrainPanel::EditTerrainPanel(ModeEditTerrain* mode) : Node<xhui::Panel>("edit-terrain-panel") {
	mode_terrain = mode;
	from_source(R"foodelim(
Dialog edit-terrain-panel ''
	Grid ? ''
		Label ? 'Terrain' big center
		---|
		Grid ? '' class=card
			Group ? 'Brush'
				ListView add-list 'a' nobar cangrabfocus=no noexpandy height=220
		---|
		Grid ? '' class=card
			Group ? 'Operations'
				Grid ? ''
					Button resize 'Resize...'
					---|
					Button load-heightmap 'Load heightmap...'
)foodelim");
	size_mode_x = SizeMode::Shrink;
	size_mode_y = SizeMode::Shrink;
	min_width_user = 320;

	event("resize", [this] {
		auto& t = mode_terrain->terrain();
		IntPairDialog::ask(this, "New terrain size", t.terrain->num_x, t.terrain->num_z).then([this] (const base::tuple<int,int>& size) {
			mode_terrain->data->execute(new ActionWorldResizeTerrain(mode_terrain->index, size.a, size.b));
		});
	});
}

