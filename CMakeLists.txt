cmake_minimum_required(VERSION 3.15)
project(RWSimplifiedPatcher VERSION 1.1)
set(CMAKE_CXX_STANDARD 20)
if (APPLE)
    # on M1 chips, this must be off to compile libxml2
    set(BUILD_SHARED_LIBS OFF)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -gdwarf-3")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -gdwarf-3")
endif ()

# STATIC stuff (Windows)
if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set(BUILD_FOR_WIN TRUE)
endif()
option(STATIC_BUILD "Build a static binary." ${BUILD_FOR_WIN})

if (STATIC_BUILD)
    set(CMAKE_EXE_LINKER_FLAGS "-static")
    set(CMAKE_FIND_LIBRARY_SUFFIXES ".a" CONFIG)
    set(BUILD_SHARED_LIBS OFF)
endif()

if (WIN32)
#    find_package(tinyxml2 CONFIG REQUIRED)
    add_subdirectory(tinyxml2)
    add_subdirectory(fmt)
#    find_package(fmt CONFIG REQUIRED)
#    find_path(vcpkg_INCLUDE_DIR NAME iconv.h PATHS $ENV{VCPKG_ROOT})
#    include_directories(${vcpkg_INCLUDE_DIR})
ELSE ()
    find_package(tinyxml2 CONFIG REQUIRED)
    find_package(fmt CONFIG REQUIRED)
    include_directories(${fmt_INCLUDE_DIR})
ENDIF ()

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/interested_tags.txt
        DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/list_tags.txt
        DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/version
        DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/example
        DESTINATION ${CMAKE_CURRENT_BINARY_DIR}
)

add_library(definfo DefInfo.cpp DefInfo.h)
target_link_libraries(definfo fmt::fmt)

add_library(xmltool xmlprocess.cpp xmlprocess.hpp)
target_link_libraries(xmltool tinyxml2::tinyxml2 definfo)

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} definfo xmltool)


