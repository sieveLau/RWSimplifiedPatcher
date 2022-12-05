#ifdef WIN32
#pragma warning(disable : 4996)
#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "../helper/helper.hpp"
#include <fmt/format.h>
#include <fstream>
#include <iostream>
#include <map>
#include <regex>
#include <string>

int main(int argc, char **argv) {

    std::string pairs_file;
    std::string search_dir;
    switch (argc) {
    case 1:
    case 2:
        std::cout << L"pairs.txt:";
        std::getline(std::cin, pairs_file);
        std::cout << L"Path to DefInjected:";
        std::getline(std::cin, search_dir);
        break;
    case 3:
        pairs_file = std::string(argv[1]);
        search_dir = std::string(argv[2]);
        break;
    default: std::wcerr << L"Too many arguments!"; exit(-1);
    }

    const static std::regex pair_pattern("(.{1,}):(.{1,})");

    std::map<std::string,std::string> translation_map;
    std::string line;
    std::ifstream pairs_file_handler;

    pairs_file_handler.open(pairs_file);
    std::string regex_replace_str;
    if (pairs_file_handler.is_open()) {
        while (std::getline(pairs_file_handler, line)) {
            std::smatch match;
            if (std::regex_match(line, match, pair_pattern)) {
                std::string key = match[1].str();
                translation_map[key] = match[2].str();
                const static std::regex pleft(R"(\()");
                const static std::regex pright(R"(\))");
                key = std::regex_replace(key, pleft, R"(\()");
                key = std::regex_replace(key, pright, R"(\))");
                regex_replace_str += fmt::format(">({0})<|", key);
            }
        }
        pairs_file_handler.close();
    }
    if (regex_replace_str.empty())
        exit(0);
    regex_replace_str.pop_back();

    std::ifstream input;

    std::regex regex_replace_re(regex_replace_str);
    for (auto &&file : file_walker(search_dir)) {
        input.open(file);
        if (input.is_open()) {
            std::string line;
            std::string output;
            while (std::getline(input, line)) {
                std::smatch match;
                if (std::regex_search(line, match, regex_replace_re)) {
                    int group_index = 1;
                    while (!match[group_index].matched)
                        group_index++;
                    output += std::regex_replace(
                        line, regex_replace_re,
                        fmt::format(">{0}<", translation_map[match[group_index].str()]));
                } else {
                    output += line;
                }
                output += '\n';
            }
            input.close();
            std::ofstream output_stream(file, std::ios_base::trunc);
            if (output_stream.is_open()) {
                output_stream << output;
                output_stream.close();
            }
        }
    }

    return 0;
}