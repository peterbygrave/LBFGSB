cmake_minimum_required (VERSION 3.7) # as earlier versions do not have VERSION_GREATER_EQUAL
project(LBFGSB CXX)

add_library(lbfgsb SHARED
        lbfgsb.hpp
        meta.h)
set_target_properties(lbfgsb PROPERTIES LINKER_LANGUAGE CXX)

add_executable(lbfgsb_app main.cpp)
target_link_libraries(lbfgsb_app lbfgsb)

find_package(Armadillo
        7.500.0
        REQUIRED
        )
if( ${ARMADILLO_FOUND} )
    message("-- Armadillo library found")
    message("     ARMADILLO_VERSION: " ${ARMADILLO_VERSION_STRING})
    message("     ARMADILLO_LIBRARIES   : " ${ARMADILLO_LIBRARIES})
    message("     ARMADILLO_INCLUDE_DIRS: " ${ARMADILLO_INCLUDE_DIRS})
endif()
if ( NOT CMAKE_BUILD_TYPE MATCHES "Debug" )
    add_definitions(-DARMA_NO_DEBUG) # disable bounds checking in Armadillo
endif()
include_directories(${ARMADILLO_INCLUDE_DIRS})
target_link_libraries(lbfgsb ${ARMADILLO_LIBRARIES})