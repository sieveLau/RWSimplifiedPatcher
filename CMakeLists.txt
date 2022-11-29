cmake_minimum_required(VERSION 3.11)
project(RWSimplifiedPatcher VERSION 1.0)
set(CMAKE_CXX_STANDARD 20)
if (APPLE)
    # on M1 chips, this must be off to compile libxml2
    set(BUILD_SHARED_LIBS OFF)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -gdwarf-3")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -gdwarf-3")
endif ()

if (WIN32)
    find_package(LibXml2 REQUIRED)
    find_package(fmt CONFIG REQUIRED)
    find_package(plog CONFIG REQUIRED)
    find_path(vcpkg_INCLUDE_DIR NAME iconv.h PATHS $ENV{VCPKG_ROOT})
    include_directories(${LIBXML2_INCLUDE_DIR} ${vcpkg_INCLUDE_DIR})
ELSE ()
    include(FetchContent)
    find_package(LibXml2 REQUIRED)
    find_package(fmt CONFIG REQUIRED)

    if (LibXml2_FOUND)
        include_directories(${LIBXML2_INCLUDE_DIR})
    ELSE ()
        FetchContent_Declare(libxml2
                URL
                https://gitlab.gnome.org/GNOME/libxml2/-/archive/v2.10.3/libxml2-v2.10.3.tar.gz
                )
        FetchContent_MakeAvailable(libxml2)
        include_directories(${libxml2_SOURCE_DIR}/include)
    ENDIF ()

    if (fmt_FOUND)
        include_directories(${fmt_INCLUDE_DIR})
    ELSE()
    FetchContent_Declare(fmt
            GIT_REPOSITORY https://github.com/fmtlib/fmt.git
            GIT_TAG 9.1.0
            )
    FetchContent_MakeAvailable(fmt)
    include_directories(${fmt_SOURCE_DIR}/include)
    ENDIF()

    FetchContent_Declare(plog
            GIT_REPOSITORY https://github.com/SergiusTheBest/plog.git
            GIT_TAG 1.1.8
            )
    FetchContent_MakeAvailable(plog)
    include_directories(${plog_SOURCE_DIR}/include)
ENDIF ()
add_subdirectory(helper)

add_executable(${PROJECT_NAME} main.cpp shared.hpp)
target_link_libraries(${PROJECT_NAME} plog::plog LibXml2::LibXml2 fmt::fmt helper)

add_subdirectory(replace)