# module: hui

HUI_BIN  = temp/hui.a
HUI_OBJ  = temp/hui.o \
 temp/hui_language.o temp/hui_config.o temp/hui_resource.o temp/hui_utility.o temp/hui_input.o \
 temp/hui_common_dlg.o temp/hui_common_dlg_gtk.o temp/hui_common_dlg_win.o \
 temp/hui_menu.o temp/hui_menu_gtk.o temp/hui_menu_win.o \
 temp/hui_window.o temp/hui_window_gtk.o temp/hui_window_win.o \
 temp/hui_window_toolbar.o temp/hui_window_toolbar_gtk.o temp/hui_window_toolbar_win.o \
 temp/hui_window_control.o temp/hui_window_control_gtk.o temp/hui_window_control_win.o
HUI_CXXFLAGS =  `pkg-config --cflags gtk+-2.0` $(GLOBALFLAGS)


$(HUI_BIN) : $(HUI_OBJ)
	rm -f $@
	ar cq $@ $(HUI_OBJ)

temp/hui.o : hui/hui.cpp
	$(CPP) -c hui/hui.cpp -o $@ $(HUI_CXXFLAGS)

temp/hui_common_dlg.o : hui/hui_common_dlg.cpp
	$(CPP) -c hui/hui_common_dlg.cpp -o $@ $(HUI_CXXFLAGS)

temp/hui_common_dlg_gtk.o : hui/hui_common_dlg_gtk.cpp
	$(CPP) -c hui/hui_common_dlg_gtk.cpp -o $@ $(HUI_CXXFLAGS)

temp/hui_common_dlg_win.o : hui/hui_common_dlg_win.cpp
	$(CPP) -c hui/hui_common_dlg_win.cpp -o $@ $(HUI_CXXFLAGS)

temp/hui_language.o : hui/hui_language.cpp
	$(CPP) -c hui/hui_language.cpp -o $@ $(HUI_CXXFLAGS)

temp/hui_input.o : hui/hui_input.cpp
	$(CPP) -c hui/hui_input.cpp -o $@ $(HUI_CXXFLAGS)

temp/hui_resource.o : hui/hui_resource.cpp
	$(CPP) -c hui/hui_resource.cpp -o $@ $(HUI_CXXFLAGS)

temp/hui_utility.o : hui/hui_utility.cpp
	$(CPP) -c hui/hui_utility.cpp -o $@ $(HUI_CXXFLAGS)

temp/hui_config.o : hui/hui_config.cpp
	$(CPP) -c hui/hui_config.cpp -o $@ $(HUI_CXXFLAGS)

temp/hui_menu.o : hui/hui_menu.cpp
	$(CPP) -c hui/hui_menu.cpp -o $@ $(HUI_CXXFLAGS)

temp/hui_menu_gtk.o : hui/hui_menu_gtk.cpp
	$(CPP) -c hui/hui_menu_gtk.cpp -o $@ $(HUI_CXXFLAGS)

temp/hui_menu_win.o : hui/hui_menu_win.cpp
	$(CPP) -c hui/hui_menu_win.cpp -o $@ $(HUI_CXXFLAGS)

temp/hui_window.o : hui/hui_window.cpp
	$(CPP) -c hui/hui_window.cpp -o $@ $(HUI_CXXFLAGS)

temp/hui_window_gtk.o : hui/hui_window_gtk.cpp
	$(CPP) -c hui/hui_window_gtk.cpp -o $@ $(HUI_CXXFLAGS)

temp/hui_window_win.o : hui/hui_window_win.cpp
	$(CPP) -c hui/hui_window_win.cpp -o $@ $(HUI_CXXFLAGS)

temp/hui_window_toolbar.o : hui/hui_window_toolbar.cpp
	$(CPP) -c hui/hui_window_toolbar.cpp -o $@ $(HUI_CXXFLAGS)

temp/hui_window_toolbar_gtk.o : hui/hui_window_toolbar_gtk.cpp
	$(CPP) -c hui/hui_window_toolbar_gtk.cpp -o $@ $(HUI_CXXFLAGS)

temp/hui_window_toolbar_win.o : hui/hui_window_toolbar_win.cpp
	$(CPP) -c hui/hui_window_toolbar_win.cpp -o $@ $(HUI_CXXFLAGS)

temp/hui_window_control.o : hui/hui_window_control.cpp
	$(CPP) -c hui/hui_window_control.cpp -o $@ $(HUI_CXXFLAGS)

temp/hui_window_control_gtk.o : hui/hui_window_control_gtk.cpp
	$(CPP) -c hui/hui_window_control_gtk.cpp -o $@ $(HUI_CXXFLAGS)

temp/hui_window_control_win.o : hui/hui_window_control_win.cpp
	$(CPP) -c hui/hui_window_control_win.cpp -o $@ $(HUI_CXXFLAGS)

hui/hui.cpp : hui/hui.h file/file.h
hui/hui_menu.cpp : hui/hui.h file/file.h
hui/hui_window.cpp : hui/hui.h file/file.h
hui/hui.h : hui/hui_config.h hui/hui_menu.h hui/hui_window.h
hui/hui_window.h : hui/hui_config.h

