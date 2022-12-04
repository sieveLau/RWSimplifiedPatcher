#pragma once
#include <codecvt>
#include <fmt/format.h>
#include <fmt/xchar.h>
#include <locale>
#include <string>
std::wstring getDirectoryPrefix();

const std::wstring &xml_header();

const std::locale &get_locale();