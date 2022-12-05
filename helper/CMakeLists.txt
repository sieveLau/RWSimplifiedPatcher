add_library(helper helper.hpp helper.cpp)
target_link_libraries(helper fmt::fmt LibXml2::LibXml2)