# module: script

SCRIPT_BIN  = temp/script.a
SCRIPT_OBJ  = temp/script.o temp/pre_script.o temp/pre_script_lexical.o \
temp/pre_script_precompiler.o temp/pre_script_preprocessor.o temp/pre_script_parser.o \
temp/script_data.o temp/script_data_file.o temp/script_data_math.o temp/script_data_threads.o \
temp/script_data_nix.o temp/script_data_hui.o temp/script_data_net.o \
temp/script_data_x.o temp/script_data_image.o temp/script_data_sound.o \
temp/script_serializer.o temp/dasm.o
SCRIPT_CXXFLAGS =  `pkg-config --cflags gtk+-2.0` $(GLOBALFLAGS)
SCRIPT_DEP =  script/script.h script/dasm.h script/pre_script.h script/script_data.h

$(SCRIPT_BIN) : $(SCRIPT_OBJ) $(SCRIPT_DEP)
	rm -f $@
	ar cq $@ $(SCRIPT_OBJ)

temp/script.o : script/script.cpp $(SCRIPT_DEP)
	$(CPP) -c script/script.cpp -o $@ $(SCRIPT_CXXFLAGS)

temp/pre_script.o : script/pre_script.cpp $(SCRIPT_DEP)
	$(CPP) -c script/pre_script.cpp -o $@ $(SCRIPT_CXXFLAGS)

temp/pre_script_lexical.o : script/pre_script_lexical.cpp $(SCRIPT_DEP)
	$(CPP) -c script/pre_script_lexical.cpp -o $@ $(SCRIPT_CXXFLAGS)

temp/pre_script_parser.o : script/pre_script_parser.cpp $(SCRIPT_DEP)
	$(CPP) -c script/pre_script_parser.cpp -o $@ $(SCRIPT_CXXFLAGS)

temp/pre_script_precompiler.o : script/pre_script_precompiler.cpp $(SCRIPT_DEP)
	$(CPP) -c script/pre_script_precompiler.cpp -o $@ $(SCRIPT_CXXFLAGS)

temp/pre_script_preprocessor.o : script/pre_script_preprocessor.cpp $(SCRIPT_DEP)
	$(CPP) -c script/pre_script_preprocessor.cpp -o $@ $(SCRIPT_CXXFLAGS)

temp/script_data.o : script/script_data.cpp $(SCRIPT_DEP)
	$(CPP) -c script/script_data.cpp -o $@ $(SCRIPT_CXXFLAGS)

temp/script_data_math.o : script/script_data_math.cpp $(SCRIPT_DEP)
	$(CPP) -c script/script_data_math.cpp -o $@ $(SCRIPT_CXXFLAGS)

temp/script_data_threads.o : script/script_data_threads.cpp $(SCRIPT_DEP)
	$(CPP) -c script/script_data_threads.cpp -o $@ $(SCRIPT_CXXFLAGS)

temp/script_data_file.o : script/script_data_file.cpp $(SCRIPT_DEP)
	$(CPP) -c script/script_data_file.cpp -o $@ $(SCRIPT_CXXFLAGS)

temp/script_data_hui.o : script/script_data_hui.cpp $(SCRIPT_DEP)
	$(CPP) -c script/script_data_hui.cpp -o $@ $(SCRIPT_CXXFLAGS)

temp/script_data_nix.o : script/script_data_nix.cpp $(SCRIPT_DEP)
	$(CPP) -c script/script_data_nix.cpp -o $@ $(SCRIPT_CXXFLAGS)

temp/script_data_net.o : script/script_data_net.cpp $(SCRIPT_DEP)
	$(CPP) -c script/script_data_net.cpp -o $@ $(SCRIPT_CXXFLAGS)

temp/script_data_x.o : script/script_data_x.cpp $(SCRIPT_DEP)
	$(CPP) -c script/script_data_x.cpp -o $@ $(SCRIPT_CXXFLAGS)

temp/script_data_image.o : script/script_data_image.cpp $(SCRIPT_DEP)
	$(CPP) -c script/script_data_image.cpp -o $@ $(SCRIPT_CXXFLAGS)

temp/script_data_sound.o : script/script_data_sound.cpp $(SCRIPT_DEP)
	$(CPP) -c script/script_data_sound.cpp -o $@ $(SCRIPT_CXXFLAGS)

temp/script_serializer.o : script/script_serializer.cpp $(SCRIPT_DEP)
	$(CPP) -c script/script_serializer.cpp -o $@ $(SCRIPT_CXXFLAGS)

temp/dasm.o : script/dasm.cpp
	$(CPP) -c script/dasm.cpp -o $@ $(SCRIPT_CXXFLAGS)

#nix/nix.cpp : nix/nix.h
#nix/nix_types.cpp : nix/nix.h
#nix/nix_sound.cpp : nix/nix.h
#nix/nix_textures.cpp : nix/nix.h
#nix/nix_net.cpp : nix/nix.h
#nix/nix.h : nix/00_config.h nix/nix_config.h nix/nix_textures.h nix/nix_types.h nix/nix_sound.h nix/nix_net.h
#nix/nix_config.h : nix/00_config.h hui/hui.h file/file.h
#nix/nix_net.h : nix/nix.h

