//
// Created by sieve on 2022/12/4.
//
#include "shared.hpp"

std::string getDirectoryPrefix() {
    const static char separator = '/';
    const static std::string prefix(
        fmt::format("Languages{0}ChineseSimplified{0}DefInjected{0}", separator));
    return prefix;
}
const std::string &xml_header() {
    const static std::string header(R"(<?xml version="1.0" encoding="utf-8"?>)");
    return header;
}
