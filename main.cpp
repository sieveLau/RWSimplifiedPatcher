#include "xmlprocess.hpp"
#include <iostream>
#include <filesystem>
#include <string>
#include <map>
#include <fstream>

std::set<std::string> get_interested_tags_from_file(const std::filesystem::path &file) {
    std::set<std::string> result;
    std::string buffer;
    std::ifstream ifile(file);
    while (std::getline(ifile, buffer)) {
        result.emplace(buffer);
    }
    return result;
}

int main(int argc, char** argv) {
    auto exec_parent = std::filesystem::path(argv[0]).parent_path();
    std::filesystem::path interested_tags_filename = "interested_tags.txt";
    std::filesystem::path list_tags_filename = "list_tags.txt";
    static auto interested_tags = get_interested_tags_from_file(exec_parent/interested_tags_filename);
    static auto list_tags = get_interested_tags_from_file(exec_parent/list_tags_filename);
    auto* r = scan_def("/home/sieve/git/RWSimplifiedPatcher/example/1127530465", interested_tags, list_tags);
    for (auto&& pair : *r) {
        std::cout << pair.first << ":\n";
        for (auto&& entry : pair.second) {
            std::cout << entry << "\n";
        }
    }
    return 0;
}