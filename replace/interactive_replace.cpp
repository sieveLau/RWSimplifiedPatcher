//
// Created by sieve on 2022/12/4.
//
#include "../helper/helper.hpp"
#include "../shared.hpp"
#include <filesystem>
#include <fmt/format.h>
#include <fmt/xchar.h>
#include <fstream>
#include <iostream>
#include <regex>
#include <string>

// TODO: ask_and_store(string,*string)
// TODO: \undo指令来reroll最近一次替换
// TODO: QT的GUI

int main() {
    using wstring = std::wstring;
    using path = std::filesystem::path;

    std::string dir_to_scan_str;

    std::cout << "Path to DefInjected:";
    std::getline(std::cin, dir_to_scan_str);
    auto src_files = file_walker(dir_to_scan_str);

    constexpr const char *regex_patter_format_string = R"([ >]({})[ <](?![^<]*>))";
    constexpr const char *regex_replace_pattern_format_string = R"(({})(?![^<]*>))";

    std::regex left_brackets(R"(\()");
    std::regex right_brackets(R"(\))");
    while (true) {

        std::string keyword;
        std::cout << "Keyword:";
        do {
            std::getline(std::cin, keyword);
        } while (keyword.empty() || keyword.starts_with('\n'));

        keyword = std::regex_replace(keyword, left_brackets, "\\(");
        keyword = std::regex_replace(keyword, right_brackets, "\\)");
        if (keyword == "exit now")
            break;
        std::string temp = fmt::format(regex_patter_format_string, keyword);
        std::regex pattern(temp);
        std::regex replace_pattern(fmt::format(regex_replace_pattern_format_string, keyword));

        std::cout << "Replace with:";
        wstring wreplacement;
        std::string replacement;
        std::wcin.imbue(get_locale());
        do {
            std::getline(std::wcin, wreplacement);
            replacement = ws2s(wreplacement);
        } while (replacement.empty() || replacement.starts_with('\n'));

        for (auto &&file : src_files) {
            std::wifstream input_file(
                R"(D:\source\RWSimplifiedPatcher\cmake-build-debug\Languages\ChineseSimplified\DefInjected\CombatExtended.AmmoCategoryDef\AmmoCategories_Neolithic.xml)");
            if (input_file.is_open()) {
                wstring line;
                wstring output_buffer;
                while (std::getline(input_file, line)) {
                    auto normal_line = ws2s(line);
                    if (std::regex_search(normal_line, pattern)) {
                        normal_line = std::regex_replace(normal_line, replace_pattern, replacement);
                    }
                    output_buffer += s2ws(normal_line);
                    output_buffer += L'\n';
                }
                input_file.close();
                std::wofstream output(file);
                output.imbue(get_locale());
                if (output.is_open()) {
                    output << output_buffer;
                    output.close();
                }
            }
        }
    }
    return 0;
}
