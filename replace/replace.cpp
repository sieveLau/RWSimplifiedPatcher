﻿#pragma warning (disable: 4996)
#include "../helper/helper.hpp"
#include <fstream>
#include <iostream>
#include <string>
#include <regex>
#include <fmt/format.h>
#include <fmt/xchar.h>
#include <plog/Log.h>
#include <filesystem>
#include <map>
#include <codecvt>
#include <windows.h>
#include <stringapiset.h>

#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
#define _CRT_SECURE_NO_WARNINGS


std::vector<std::filesystem::path> file_walker(const std::wstring &dir,
                                               const std::wstring &extension = L".xml") {
    std::vector<std::filesystem::path> result;
    using recursive_directory_iterator = std::filesystem::recursive_directory_iterator;
    for (const auto &dirEntry : recursive_directory_iterator(dir))
        if (std::filesystem::is_regular_file(dirEntry.path())) {
            if (dirEntry.path().filename().extension() == extension) {
                result.push_back(dirEntry.path());
                PLOGD << "source file found: " << dirEntry.path().filename();
            }
        }
    return result;
}

int main(int argc, char **argv) {
    const std::locale utf8( std::locale(), new std::codecvt_utf8<wchar_t> );
    std::wcout.imbue(utf8);
    std::wcin.imbue(utf8);


    std::wstring pairs_file;
    std::wstring search_dir;
    switch (argc) {
    case 1:
    case 2:
        std::wcout << "pairs.txt:";
        std::getline(std::wcin, pairs_file);
        std::wcout << "Path to DefInjected:";
        std::getline(std::wcin, search_dir);
        break;
    case 3:
        pairs_file = s2ws(std::string(argv[1]));
        search_dir = s2ws(std::string(argv[2]));
        break;
    default: std::cerr << "Too many arguments!"; exit(-1);
    }

    const static std::wregex pair_pattern(L"(.{1,}):(.{1,})");
    //std::wstring line;
    //std::getline(std::wcin, line);


    std::map<std::wstring,std::wstring> translation_map;
    std::wstring line;
    std::wifstream pairs_file_handler;
    pairs_file_handler.imbue(utf8);
    pairs_file_handler.open(pairs_file);
    std::wstring regex_replace_str;
    if (pairs_file_handler.is_open()) {
        while(std::getline(pairs_file_handler,line)) {
            std::wsmatch match;
            if (std::regex_match(line, match, pair_pattern)) {
                std::wstring key = match[1].str();
                translation_map[key]=match[2].str();
                const static std::wregex pleft(LR"(\()");
                const static std::wregex pright(LR"(\))");
                key = std::regex_replace(key,pleft,LR"(\()");
                key = std::regex_replace(key,pright,LR"(\))");
                regex_replace_str+=fmt::format(L">({0})<|",key);
            }
        }
        pairs_file_handler.close();
    }
    if(regex_replace_str.empty())exit(0);
    regex_replace_str.pop_back();

    std::wifstream input;
    input.imbue(utf8);
    std::wregex regex_replace_re(regex_replace_str);
    for (auto&& file : file_walker(search_dir))
    {
        input.open(file);
        if(input.is_open()){
            std::wstring line;
            std::wstring output;
            while(std::getline(input,line)){
                std::wsmatch match;
                if(std::regex_search(line,match,regex_replace_re)){
                    int group_index = 1;
                    while(match[group_index].matched==false)group_index++;
                    output+=std::regex_replace(line,regex_replace_re,fmt::format(L">{0}<",translation_map[match[group_index].str()]));
                }else{
                    output+=line;
                }
                output+='\n';
            }
            input.close();
            std::wofstream output_stream(file,std::ios_base::trunc);
            output_stream.imbue(utf8);
            if(output_stream.is_open()){
                output_stream<<output;
                output_stream.close();
            }
        }
    }

    return 0;
}