#pragma once

#include "Window.h"
#include "Painter.h"
#include "../os/path.h"
#include "../base/map.h"
#include <lib/ygraphics/graphics-fwd.h>

namespace font {
	struct Face;
}

class Image;

namespace xhui {

void init(const Array<string> &arg, const string& app_name);
void run();

	extern float global_ui_scale;
	extern ColorSpace color_space_display;
	extern ColorSpace color_space_shaders;
	extern ColorSpace color_space_input;


enum class Flags {
	NONE = 0,
	DONT_LOAD_RESOURCE = 1,
	SILENT = 2,
	NO_ERROR_HANDLER = 4,
	UNIQUE = 16,
	OWN_DECORATION = 64,
	FAKE = 128
};
Flags operator|(Flags a, Flags b);
int operator&(Flags a, Flags b);


int run_repeated(float dt, Callback f);
int run_later(float dt, Callback f);
void cancel_runner(int id);

namespace clipboard {
	void copy(const string& text);
	string paste();
}

// key codes (physical keys)
enum {
	KEY_LCONTROL,
	KEY_RCONTROL,
	KEY_LSHIFT,
	KEY_RSHIFT,
	KEY_LALT,
	KEY_RALT,
	KEY_PLUS,
	KEY_MINUS,
	KEY_FENCE,		// "Raute"???
	KEY_END,
	KEY_PAGE_UP,
	KEY_PAGE_DOWN,
	KEY_UP,
	KEY_DOWN,
	KEY_LEFT,
	KEY_RIGHT,
	KEY_RETURN,
	KEY_ESCAPE,
	KEY_INSERT,
	KEY_DELETE,
	KEY_SPACE,
	KEY_F1,
	KEY_F2,
	KEY_F3,
	KEY_F4,
	KEY_F5,
	KEY_F6,
	KEY_F7,
	KEY_F8,
	KEY_F9,
	KEY_F10,
	KEY_F11,
	KEY_F12,
	KEY_0,
	KEY_1,
	KEY_2,
	KEY_3,
	KEY_4,
	KEY_5,
	KEY_6,
	KEY_7,
	KEY_8,
	KEY_9,
	KEY_A,
	KEY_B,
	KEY_C,
	KEY_D,
	KEY_E,
	KEY_F,
	KEY_G,
	KEY_H,
	KEY_I,
	KEY_J,
	KEY_K,
	KEY_L,
	KEY_M,
	KEY_N,
	KEY_O,
	KEY_P,
	KEY_Q,
	KEY_R,
	KEY_S,
	KEY_T,
	KEY_U,
	KEY_V,
	KEY_W,
	KEY_X,
	KEY_Y,
	KEY_Z,
	KEY_BACKSPACE,
	KEY_TAB,
	KEY_HOME,
	KEY_NUM_0,
	KEY_NUM_1,
	KEY_NUM_2,
	KEY_NUM_3,
	KEY_NUM_4,
	KEY_NUM_5,
	KEY_NUM_6,
	KEY_NUM_7,
	KEY_NUM_8,
	KEY_NUM_9,
	KEY_NUM_ADD,
	KEY_NUM_SUBTRACT,
	KEY_NUM_MULTIPLY,
	KEY_NUM_DIVIDE,
	KEY_NUM_COMMA,
	KEY_NUM_ENTER,
	KEY_COMMA,
	KEY_DOT,
	KEY_LESS,
	KEY_SZ,
	KEY_AE,
	KEY_OE,
	KEY_UE,
	KEY_GRAVE,
	KEY_LWINDOWS,
	KEY_RWINDOWS,

	NUM_KEYS,

	KEY_ANY,
	KEY_CONTROL = 256,
	KEY_SHIFT = 512,
	KEY_ALT = 1024,
	KEY_SUPER = 2048 // mac: command
};

namespace event_id {
	extern const string Activate;
	extern const string ActivateDialogDefault;
	extern const string Close;
	extern const string Click;
	extern const string Changed;
	extern const string MouseMove;
	extern const string MouseEnter;
	extern const string MouseLeave;
	extern const string MouseWheel;
	extern const string Draw;
	extern const string Initialize;
	extern const string JustBeforeDraw; // for y renderer.prepare()
	extern const string LeftButtonDown;
	extern const string LeftButtonUp;
	extern const string MiddleButtonDown;
	extern const string MiddleButtonUp;
	extern const string RightButtonDown;
	extern const string RightButtonUp;
	extern const string KeyDown;
	extern const string KeyUp;
	extern const string KeyChar;
	extern const string Select;
	extern const string DragStart;
	extern const string DragDrop;
	extern const string Scroll;
};

struct XImage {
	string uid;
	Path filename;
	owned<::Image> image;
	shared<ygfx::Texture> texture;
	bool dirty = true;
	vec2 size() const;
};

XImage* load_image(const string& name);
string create_image(const ::Image& im);
void set_image(const string& uid, const ::Image& im);
string texture_to_image(const shared<ygfx::Texture>& texture);
void delete_image(const string& name);
void prepare_image(XImage* image);


extern font::Face* default_font_regular;
extern font::Face* default_font_bold;
extern font::Face* default_font_mono_regular;
extern font::Face* default_font_mono_bold;
font::Face* pick_font(const string &font, bool bold, bool italic);

}
