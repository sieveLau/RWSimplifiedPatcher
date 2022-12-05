add_executable(replace replace.cpp)
target_link_libraries(replace fmt::fmt helper)

if (WIN32)
ELSE ()
    add_executable(ireplace interactive_replace.cpp)
    target_link_libraries(ireplace fmt::fmt helper shared)
ENDIF ()