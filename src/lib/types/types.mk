# module: types

TYPES_BIN  = temp/types.a
TYPES_OBJ  = temp/types.o temp/types_color.o temp/types_color.o temp/types_complex.o temp/types_vector.o \
   temp/types_matrix.o temp/types_matrix3.o temp/types_quaternion.o temp/types_plane.o \
   temp/types_rect.o temp/types_interpolation.o
TYPES_CXXFLAGS = $(GLOBALFLAGS)

$(TYPES_BIN) : $(TYPES_OBJ)
	rm -f $@
	ar cq $@ $(TYPES_OBJ)

temp/types.o : types/types.cpp
	$(CPP) -c types/types.cpp -o $@ $(TYPES_CXXFLAGS)

temp/types_color.o : types/color.cpp
	$(CPP) -c types/color.cpp -o $@ $(TYPES_CXXFLAGS)

temp/types_complex.o : types/complex.cpp
	$(CPP) -c types/complex.cpp -o $@ $(TYPES_CXXFLAGS)

temp/types_vector.o : types/vector.cpp
	$(CPP) -c types/vector.cpp -o $@ $(TYPES_CXXFLAGS)

temp/types_matrix.o : types/matrix.cpp
	$(CPP) -c types/matrix.cpp -o $@ $(TYPES_CXXFLAGS)

temp/types_matrix3.o : types/matrix3.cpp
	$(CPP) -c types/matrix3.cpp -o $@ $(TYPES_CXXFLAGS)

temp/types_quaternion.o : types/quaternion.cpp
	$(CPP) -c types/quaternion.cpp -o $@ $(TYPES_CXXFLAGS)

temp/types_plane.o : types/plane.cpp
	$(CPP) -c types/plane.cpp -o $@ $(TYPES_CXXFLAGS)

temp/types_rect.o : types/rect.cpp
	$(CPP) -c types/rect.cpp -o $@ $(TYPES_CXXFLAGS)

temp/types_interpolation.o : types/interpolation.cpp
	$(CPP) -c types/interpolation.cpp -o $@ $(TYPES_CXXFLAGS)
