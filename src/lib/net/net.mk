# module: net

NET_BIN  = temp/net.a
NET_OBJ  = temp/net.o
NET_CXXFLAGS =  $(GLOBALFLAGS)


$(NET_BIN) : $(NET_OBJ)
	rm -f $@
	ar cq $@ $(NET_OBJ)

temp/net.o : net/net.cpp
	$(CPP) -c net/net.cpp -o $@ $(NET_CXXFLAGS)
