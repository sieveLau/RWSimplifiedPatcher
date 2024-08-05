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

void entry(const std::filesystem::path& input_dir, const std::filesystem::path& output_dir,  const std::filesystem::path& exec_dir, const std::string& version) {
    using std::filesystem::path;
    using std::filesystem::exists;
    using std::filesystem::create_directories;
    using std::map;
    using std::set;
    using std::filesystem::filesystem_error;
    using std::string;
    using std::filesystem::absolute;

    const static std::filesystem::path k_interested_tags_filename = "interested_tags.txt";
    const static std::filesystem::path k_list_tags_filename = "list_tags.txt";
    const static auto interested_tags = get_interested_tags_from_file(exec_dir/k_interested_tags_filename);
    const static auto list_tags = get_interested_tags_from_file(exec_dir/k_list_tags_filename);

    if (!exists(input_dir)) {
        throw filesystem_error("No such file or directory", input_dir, std::error_code());
    }
    set<string> possible_def_subdir {fmt::format("v{}", version), fmt::format("{}", version)};
    set<path> possible_def_dir {absolute(input_dir/"Defs")};
    for (const auto & subdir : possible_def_subdir) {
        possible_def_dir.emplace(absolute(input_dir/subdir/"Defs"));
    }
    std::unique_ptr<std::map<std::basic_string<char>, std::list<rimtrans::DefInfo>>> def_map_by_class(new std::map<std::basic_string<char>, std::list<rimtrans::DefInfo>>);
    for (const auto & dir : possible_def_dir) {
        if (!exists(dir))
            continue;
        scan_def(dir, interested_tags, list_tags, def_map_by_class.get());
    }
    format_defs_to_file(output_dir, *def_map_by_class);
}

int main(int argc, char** argv) {
    auto exec_parent = std::filesystem::path(argv[0]).parent_path();
    entry("/home/sieve/git/RWSimplifiedPatcher/example/1127530465",
          "/home/sieve/git/RWSimplifiedPatcher/example/Test_112",
          exec_parent,
          "1.5"
          );

    return 0;
}