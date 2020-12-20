#https://gist.github.com/hbirchtree/afada1b4886a79abac3e2771773059e3

cmake_minimum_required(VERSION 3.12)

# set the project name
project(Edward)

# c++14
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)



find_package(PkgConfig REQUIRED)
PKG_CHECK_MODULES(GTK REQUIRED gtk+-3.0)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)





# Setup CMake to use GTK+, tell the compiler where to look for headers
# and to the linker where to look for libraries
include_directories(${GTK_INCLUDE_DIRS})
link_directories(${GTK_LIBRARY_DIRS})

# Add other flags to the compiler
add_definitions(${GTK_CFLAGS_OTHER})

# Add an executable compiled from hello.c
file(GLOB_RECURSE src "src/*.cpp")
add_executable(edward)

add_compile_definitions(HAS_LIB_GTK3=1 HAS_LIB_ZLIB=1 HAS_LIB_GL=1 HAS_LIB_UNWIND=1 HAS_LIB_DL=1)

#message(STATUS ${src})
target_sources(edward PUBLIC ${src})


# Link the target to the GTK+ libraries
target_link_libraries(edward ${GTK_LIBRARIES} Threads::Threads dl unwind GL z)

