//
// Created by sieve on 2022/12/4.
//
#include "shared.hpp"

std::wstring getDirectoryPrefix() {
#ifdef WIN32
    const static wchar_t separator = L'\\';
#else
    const static wchar_t separator = L'/';
#endif
    const static std::wstring prefix(
        fmt::format(L"Languages{0}ChineseSimplified{0}DefInjected{0}", separator));
    return prefix;
}
const std::wstring &xml_header() {
    const static std::wstring header(LR"(<?xml version="1.0" encoding="utf-8"?>)");
    return header;
}
const std::locale &get_locale() {
    const static std::locale utf8(std::locale(), new std::codecvt_utf8<wchar_t>);
    return utf8;
}
