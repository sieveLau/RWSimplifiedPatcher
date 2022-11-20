cmake_minimum_required(VERSION 3.11)
project(RWSimplifiedPatcher VERSION 1.0)
include(FetchContent)
set(CMAKE_CXX_STANDARD 20)
# on M1 chips, this must be off to compile libxml2
set(BUILD_SHARED_LIBS OFF)
#set(ENABLE_LIBCXX OFF)
if (APPLE)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -gdwarf-3")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -gdwarf-3")
endif ()


FetchContent_Declare(libxml2
        URL
        https://gitlab.gnome.org/GNOME/libxml2/-/archive/v2.10.3/libxml2-v2.10.3.tar.gz
        )
FetchContent_MakeAvailable(libxml2)
include_directories(${libxml2_SOURCE_DIR}/include)

FetchContent_Declare(fmt
        GIT_REPOSITORY https://github.com/fmtlib/fmt.git
        GIT_TAG 9.1.0
        )
FetchContent_MakeAvailable(fmt)
include_directories(${fmt_SOURCE_DIR}/include)

FetchContent_Declare(plog
        GIT_REPOSITORY https://github.com/SergiusTheBest/plog.git
        GIT_TAG 1.1.8
        )
FetchContent_MakeAvailable(plog)
include_directories(${plog_SOURCE_DIR}/include)

# add_library(simplexml simple_xml.h simple_xml.cc)
# add_library(tools tools.cpp tools.h)
# target_link_libraries(simplexml fmt::fmt LibXml2::LibXml2)
# add_executable(RWSimplifiedPatcher consts.h main.cpp)
# target_link_libraries(RWSimplifiedPatcher fmt::fmt simplexml tools LibXml2::LibXml2 plog::plog)
add_library(helper helper.hpp helper.cpp)
add_executable(main test.cpp)
target_link_libraries(main plog::plog LibXml2::LibXml2 fmt::fmt helper)