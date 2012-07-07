# module: x

X_BIN  = temp/x.a
X_OBJ  = temp/meta.o temp/fx.o temp/camera.o temp/model.o temp/object.o temp/terrain.o\
temp/god.o temp/links.o temp/matrixn.o temp/tree.o temp/collision.o temp/physics.o temp/gui.o
X_CXXFLAGS = `pkg-config --cflags gtk+-3.0` $(GLOBALFLAGS)

$(X_BIN) : $(X_OBJ)
	rm -f $@
	ar cq $@ $(X_OBJ)

temp/camera.o : x/camera.cpp
	$(CPP) -c x/camera.cpp -o $@ $(X_CXXFLAGS)

temp/collision.o : x/collision.cpp
	$(CPP) -c x/collision.cpp -o $@ $(X_CXXFLAGS)

temp/fx.o : x/fx.cpp
	$(CPP) -c x/fx.cpp -o $@ $(X_CXXFLAGS)

temp/god.o : x/god.cpp
	$(CPP) -c x/god.cpp -o $@ $(X_CXXFLAGS)

temp/links.o : x/links.cpp
	$(CPP) -c x/links.cpp -o $@ $(X_CXXFLAGS)

temp/matrixn.o : x/matrixn.cpp
	$(CPP) -c x/matrixn.cpp -o $@ $(X_CXXFLAGS)

temp/meta.o : x/meta.cpp
	$(CPP) -c x/meta.cpp -o $@ $(X_CXXFLAGS) -O1

temp/model.o : x/model.cpp
	$(CPP) -c x/model.cpp -o $@ $(X_CXXFLAGS)

temp/object.o : x/object.cpp
	$(CPP) -c x/object.cpp -o $@ $(X_CXXFLAGS)

temp/physics.o : x/physics.cpp
	$(CPP) -c x/physics.cpp -o $@ $(X_CXXFLAGS)

temp/terrain.o : x/terrain.cpp
	$(CPP) -c x/terrain.cpp -o $@ $(X_CXXFLAGS)

temp/tree.o : x/tree.cpp
	$(CPP) -c x/tree.cpp -o $@ $(X_CXXFLAGS)

temp/gui.o : x/gui.cpp
	$(CPP) -c x/gui.cpp -o $@ $(X_CXXFLAGS)


x/camera.cpp : x/x.h
x/collision.cpp : x/x.h
x/fx.cpp : x/x.h
x/god.cpp : x/x.h
x/links.cpp : x/x.h
x/matrixn.cpp : x/x.h
x/meta.cpp : x/x.h
x/model.cpp : x/x.h
x/object.cpp : x/x.h
x/physics.cpp :
x/terrain.cpp : x/x.h
x/tree.cpp : x/x.h
x/x.h : nix/nix.h file/file.h x/camera.h x/collision.h x/model.h x/god.h x/links.h x/physics.h x/terrain.h x/object.h x/tree.h x/matrixn.h x/fx.h
