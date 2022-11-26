#include <fstream>
#include <iostream>
#include <string>
#include <regex>
#include <fmt/format.h>
#include <plog/Log.h>
#include <filesystem>
#include <map>

std::vector<std::filesystem::path> file_walker(const std::string &dir,
                                               const std::string &extension = ".xml") {
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
    std::regex pair_pattern("(.{1,}):(.{1,})");

    std::cout << "pairs.txt: ";
    std::string line;
    std::getline(std::cin, line);

    std::string search_dir;
    std::cout << "Path to DefInjected: ";
    std::getline(std::cin, search_dir);

    std::map<std::string,std::string> translation_map;
    std::ifstream pairs_file_handler;
    pairs_file_handler.open(line);
    std::string regex_replace_str;
    if (pairs_file_handler.is_open()) {
        while(std::getline(pairs_file_handler,line)) {
            std::smatch match;
            if (std::regex_match(line, match, pair_pattern)) {
                std::string key = match[1].str();
                translation_map[key]=match[2].str();
                std::regex pleft(R"(\()");
                std::regex pright(R"(\))");
                key = std::regex_replace(key,pleft,R"(\()");
                key = std::regex_replace(key,pright,R"(\))");
                regex_replace_str+=fmt::format(">({0})<|",key);
            }
        }
        pairs_file_handler.close();
    }
    if(regex_replace_str.empty())exit(0);
    regex_replace_str.pop_back();

    std::ifstream input;
    std::regex regex_replace_re(regex_replace_str);
    for (auto&& file : file_walker(search_dir))
    {
        input.open(file);
        if(input.is_open()){
            std::string line;
            std::string output;
            while(std::getline(input,line)){
                std::smatch match;
                if(std::regex_search(line,match,regex_replace_re)){
                    int group_index = 1;
                    while(match[group_index].matched==false)group_index++;
                    output+=std::regex_replace(line,regex_replace_re,fmt::format(">{0}<",translation_map[match[group_index].str()]));
                }else{
                    output+=line;
                }
                output+='\n';
            }
            input.close();
            std::ofstream output_stream(file,std::ios_base::trunc);
            if(output_stream.is_open()){
                output_stream<<output;
                output_stream.close();
            }
        }
    }

    return 0;
}