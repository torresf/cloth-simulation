cmake_minimum_required(VERSION 2.8)

project(flag_simulation)

add_definitions(-std=c++11)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/CMake)

find_package(SDL REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)

# Pour gérer un bug a la fac, a supprimer sur machine perso:
#set(OPENGL_LIBRARIES /usr/lib/x86_64-linux-gnu/libGL.so.1)

include_directories(${SDL_INCLUDE_DIR} ${OPENGL_INCLUDE_DIR} ${GLEW_INCLUDE_DIR} PartyKel/include third-party/AntTweakBar/include third-party/include)

add_subdirectory(PartyKel)
add_subdirectory(third-party/AntTweakBar)

set(ALL_LIBRARIES PartyKel AntTweakBar ${SDL_LIBRARY} ${OPENGL_LIBRARIES} ${GLEW_LIBRARY})

file(GLOB_RECURSE SRC_FILES src/*.cpp)

foreach(SRC_FILE ${SRC_FILES})
    get_filename_component(FILE ${SRC_FILE} NAME_WE)
    add_executable(${FILE} ${SRC_FILE})
    target_link_libraries(${FILE} ${ALL_LIBRARIES})
endforeach()
