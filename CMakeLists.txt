cmake_minimum_required(VERSION 3.10)
project("yasli")

if (MSVC)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
elseif(ANDROID)
	add_definitions(-DANDROID_NDK)
endif()

if (CMAKE_CXX_COMPILER MATCHES "clang")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations -Wno-unused-value")
endif()

if (UNIX)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

mark_as_advanced(CMAKE_INSTALL_PREFIX)
mark_as_advanced(QT_QMAKE_EXECUTABLE)

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DNDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG")

if (MSVC)
  add_link_options(/DEBUG)
endif (MSVC)

#set(PROJECT_SOURCE_ROOT ${PROJECT_SOURCE_DIR})
#set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_ROOT})
#set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_ROOT})

#add_subdirectory(UnitTest++)
#include_directories(UnitTest++/src)

include_directories(.)
add_subdirectory(yasli)
add_subdirectory(yasli-example)
#add_subdirectory(yasli-test)

if(NOT YASLI_NO_QT)
	add_subdirectory(QPropertyTree)
	add_subdirectory(QPropertyTree-example)
endif()

if (MSVC)
  add_subdirectory(ww)
  add_subdirectory(ww-example)
endif (MSVC)

