cmake_minimum_required (VERSION 2.9)

project (vouw)

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
if(COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
else()
    message(FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O3")

#set( CMAKE_VERBOSE_MAKEFILE ON )


add_library (vouw STATIC
    src/vouw/vouw.cpp
    src/vouw/matrix.cpp
    src/vouw/pattern.cpp
    src/vouw/instance.cpp
    src/vouw/instance_matrix.cpp
    src/vouw/codetable.cpp
    src/vouw/equivalence.cpp
    src/vouw/massfunction.cpp
    src/vouw/encoder.cpp
    src/vouw/noisy_equivalence.cpp)

#add_executable (image-vouw 
#    src/image-vouw/main.c)

target_link_libraries (vouw "-lm")
#target_link_libraries (image-vouw vouw)
target_include_directories (vouw PRIVATE "include")
#target_include_directories (image-vouw PRIVATE "include")

##
## Build configuration for the QVouw tool
## 

# Find the QtWidgets library
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
find_package(Qt5Widgets)

if( Qt5Widgets_FOUND )
    add_executable( qvouw 
        src/qvouw/main.cpp
        src/qvouw/vouw.cpp
        src/qvouw/qvouw.qrc
        src/qvouw/vouwwidget.cpp
        src/qvouw/matrixwidget.cpp
        src/qvouw/mainwindow.cpp
        src/qvouw/vouwitemmodel.cpp)

# Use the Widgets module from Qt 5.
    target_link_libraries(qvouw Qt5::Widgets)
    target_link_libraries (qvouw vouw)
    target_include_directories (qvouw PRIVATE "include")

endif()
