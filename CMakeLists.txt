cmake_minimum_required(VERSION 3.15)
project(RWSimplifiedPatcher VERSION 1.1)
set(CMAKE_CXX_STANDARD 20)
if (APPLE)
    # on M1 chips, this must be off to compile libxml2
    set(BUILD_SHARED_LIBS OFF)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -gdwarf-3")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -gdwarf-3")
endif ()

if (WIN32)
    find_package(tinyxml2 REQUIRED)
    find_package(fmt CONFIG REQUIRED)
    find_path(vcpkg_INCLUDE_DIR NAME iconv.h PATHS $ENV{VCPKG_ROOT})
    include_directories(${vcpkg_INCLUDE_DIR})
ELSE ()
    include(FetchContent)
    find_package(tinyxml2 REQUIRED)
    find_package(fmt CONFIG REQUIRED)
    include_directories(${fmt_INCLUDE_DIR})
ENDIF ()

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/interested_tags.txt
        DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/list_tags.txt
        DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

add_library(definfo DefInfo.cpp DefInfo.h)
target_link_libraries(definfo fmt::fmt)

add_library(xmltool xmlprocess.cpp xmlprocess.hpp)
target_link_libraries(xmltool tinyxml2::tinyxml2 definfo)

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} definfo xmltool)


