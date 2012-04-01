/*----------------------------------------------------------------------------*\
| CHui                                                                         |
| -> Heroic User Interface                                                     |
| -> abstraction layer for GUI                                                 |
|   -> Windows (default api) or Linux (Gtk+)                                   |
|                                                                              |
| vital properties:                                                            |
|                                                                              |
| last updated: 2009.12.05 (c) by MichiSoft TM                                 |
\*----------------------------------------------------------------------------*/

#ifndef _HUI_EXISTS_
#define _HUI_EXISTS_

#include "hui_common.h"
#include "../types/types.h"
#ifdef _X_USE_IMAGE_
	#include "../image/image.h"
#else
	typedef int Image;
#endif

extern string HuiVersion;




class CHuiWindow;
class CHuiMenu;
class HuiEventHandler;






//----------------------------------------------------------------------------------
// system independence of main() function

extern Array<string> HuiMakeArgs(int num_args, char *args[]);



// for a system independent usage of this library

#define hui_main(p)	\
_hui_main_(p);\
int main(int NumArgs, char *Args[])\
{\
	return _hui_main_(HuiMakeArgs(NumArgs, Args));\
}\
int _hui_main_(p)

// usage:
//
// int hui_main(Array<string> arg)
// {
//     HuiInit();
//     ....
//     return HuiRun();
// }




//----------------------------------------------------------------------------------
// hui itself


// execution
void HuiInit();
void HuiInitExtended(const string &program, const string &version, hui_callback *error_cleanup_function, bool load_res, const string &def_lang);
int HuiRun();
void HuiPushMainLevel();
void HuiPopMainLevel();
void HuiSetIdleFunction(hui_callback *idle_function);
void HuiSetIdleFunctionM(HuiEventHandler *object, void (HuiEventHandler::*function)());
void HuiRunLater(int time_ms, hui_callback *function);
void HuiRunLaterM(int time_ms, HuiEventHandler *object, void (HuiEventHandler::*function)());
void HuiDoSingleMainLoop();
void HuiEnd();
string HuiWaitTillWindowClosed(CHuiWindow *win);
extern bool HuiEndKeepMsgAlive;

// images
//int HuiLoadImage(const string &filename);
string HuiSetImage(const Image &image);

// HUI configuration
extern char HuiComboBoxSeparator, *HuiSeparator;
extern bool HuiCreateHiddenWindows;

// data from hui (...don't change...)
extern string HuiAppFilename, HuiAppDirectory, HuiAppDirectoryStatic;
extern string HuiInitialWorkingDirectory;
extern bool HuiRunning;



#include "hui_menu.h"
#include "hui_window.h"
#include "hui_common_dlg.h"
#include "hui_language.h"
#include "hui_config.h"
#include "hui_input.h"
#include "hui_resource.h"
#include "hui_utility.h"


#endif


