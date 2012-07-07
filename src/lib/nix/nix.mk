# module: nix

NIX_BIN  = temp/nix.a
NIX_OBJ  = temp/nix.o temp/nix_draw.o temp/nix_view.o temp/nix_input.o temp/nix_textures.o \
temp/nix_light.o temp/nix_shader.o temp/nix_vertexbuffer.o
NIX_CXXFLAGS =  `pkg-config --cflags gtk+-3.0` $(GLOBALFLAGS)

$(NIX_BIN) : $(NIX_OBJ)
	rm -f $@
	ar cq $@ $(NIX_OBJ)

temp/nix.o : nix/nix.cpp
	$(CPP) -c nix/nix.cpp -o $@ $(NIX_CXXFLAGS)

temp/nix_draw.o : nix/nix_draw.cpp
	$(CPP) -c nix/nix_draw.cpp -o $@ $(NIX_CXXFLAGS)

temp/nix_view.o : nix/nix_view.cpp
	$(CPP) -c nix/nix_view.cpp -o $@ $(NIX_CXXFLAGS)

temp/nix_light.o : nix/nix_light.cpp
	$(CPP) -c nix/nix_light.cpp -o $@ $(NIX_CXXFLAGS)

temp/nix_shader.o : nix/nix_shader.cpp
	$(CPP) -c nix/nix_shader.cpp -o $@ $(NIX_CXXFLAGS)

temp/nix_vertexbuffer.o : nix/nix_vertexbuffer.cpp
	$(CPP) -c nix/nix_vertexbuffer.cpp -o $@ $(NIX_CXXFLAGS)

temp/nix_input.o : nix/nix_input.cpp
	$(CPP) -c nix/nix_input.cpp -o $@ $(NIX_CXXFLAGS)

temp/nix_textures.o : nix/nix_textures.cpp
	$(CPP) -c nix/nix_textures.cpp -o $@ $(NIX_CXXFLAGS)

nix/nix.cpp : nix/nix.h
nix/nix_textures.cpp : nix/nix.h
nix/nix_draw.cpp : nix/nix.h
nix/nix_view.cpp : nix/nix.h
nix/nix_input.cpp : nix/nix.h
nix/nix_net.cpp : nix/nix.h
nix/nix.h : 00_config.h nix/nix_config.h nix/nix_draw.h nix/nix_view.h nix/nix_input.h nix/nix_textures.h
nix/nix_config.h : 00_config.h hui/hui.h file/file.h
#nix/nix_net.h : nix/nix.h

