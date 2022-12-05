//
// Created by sieve on 2022/12/4.
//
#include "../helper/helper.hpp"
#include "../shared.hpp"
#include <filesystem>
#include <fmt/format.h>
#include <fstream>
#include <iostream>
#include <regex>
#include <string>

// TODO: ask_and_store(string,*string)
// TODO: \undo指令来reroll最近一次替换
// TODO: QT的GUI

int main() {
    using path = std::filesystem::path;

    std::string dir_to_scan_str;

    std::cout << "Path to DefInjected:";
    std::getline(std::cin, dir_to_scan_str);
    auto src_files = file_walker(dir_to_scan_str);

    constexpr const char *regex_patter_format_string = R"([ >]{}[ <](?!<*>))";
    constexpr const char *regex_replace_pattern_format_string = R"(({})(?!<*>))";

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
        std::string replacement;
        do {
            std::getline(std::cin, replacement);
        } while (replacement.empty() || replacement.starts_with('\n'));

        for (auto &&file : src_files) {
            std::ifstream input_file(file);
            if (input_file.is_open()) {
                std::string line;
                std::string output_buffer;
                while (std::getline(input_file, line)) {
                    if (std::regex_search(line, pattern)) {
                        line = std::regex_replace(line.c_str(), replace_pattern, replacement);
                    }
                    output_buffer += line;
                    output_buffer += '\n';
                }
                input_file.close();
                std::ofstream output(file);
                if (output.is_open()) {
                    output << output_buffer;
                    output.close();
                }
            }
        }
    }
    return 0;
}
