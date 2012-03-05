# module: threads

THREAD_BIN  = temp/threads.a
THREAD_OBJ  = temp/threads.o temp/mutex.o temp/work.o
THREAD_CXXFLAGS = `pkg-config --cflags gtk+-2.0` $(GLOBALFLAGS)
THREAD_DEP =  threads/threads.h

$(THREAD_BIN) : $(THREAD_OBJ) $(THREAD_DEP)
	rm -f $@
	ar cq $@ $(THREAD_OBJ)

temp/threads.o : threads/threads.cpp $(THREAD_DEP)
	$(CPP) -c threads/threads.cpp -o $@ $(THREAD_CXXFLAGS)

temp/work.o : threads/work.cpp $(THREAD_DEP)
	$(CPP) -c threads/work.cpp -o $@ $(THREAD_CXXFLAGS)

temp/mutex.o : threads/mutex.cpp $(THREAD_DEP)
	$(CPP) -c threads/mutex.cpp -o $@ $(THREAD_CXXFLAGS)


