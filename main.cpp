#include "xmlprocess.hpp"
#include <iostream>
#include <filesystem>
#include <string>
#include <map>
#include <fstream>
#include <regex>

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
    if (exists(output_dir) && equivalent(input_dir, output_dir)){
        throw std::invalid_argument("I/O path is the same");
    }

    auto_about(output_dir, input_dir, version);

    set<string> possible_def_subdir {fmt::format("v{}", version), fmt::format("{}", version)};
    set<path> possible_def_dir {absolute(input_dir/"Defs")};
    for (const auto & subdir : possible_def_subdir) {
        possible_def_dir.emplace(absolute(input_dir/subdir/"Defs"));
    }
    std::unique_ptr<std::map<std::basic_string<char>, std::list<rimtrans::DefInfo>>> def_map_by_class(new std::map<std::basic_string<char>, std::list<rimtrans::DefInfo>>);
    for (const auto & dir : possible_def_dir) {
        if (!exists(dir))
            continue;
        scan_def(dir.string(), interested_tags, list_tags, def_map_by_class.get());
    }
    format_defs_to_file(output_dir, *def_map_by_class);
    copy_included_trans(output_dir, input_dir);
}

int main(int argc, char** argv) {
    using std::filesystem::path;
    using std::filesystem::exists;
    using std::string;
    using std::regex;
    using std::regex_match;

    if (argc < 3) {
        printf("Usage: RWSimplifiedPatcher <input_dir> <output_dir>");
        exit(1);
    }

    path input_dir(argv[1]);
    if (!exists(input_dir)) {
        printf("[FATAL]Input dir %s not found.", std::filesystem::absolute(input_dir).string().c_str());
        exit(2);
    }

    const static char* FMT_STR_INVALID_CONFIG_VERSION_NUM = "[WARN]Invalid version in %s, ignored. Using %s as version.";
    const static char* FMT_STR_NO_VERSION_FILE = "[WARN]Version config file %s not found, using %s as version.";
    const static char* FMT_STR_SUCCESS_VERSION_READ = "[INFO]Version config file %s valid, using %s as version.";

    const auto exec_parent = std::filesystem::path(argv[0]).parent_path();
    path version_file = exec_parent/"version";
    const static char* k_hardcoded_version = "1.5";
    string version(k_hardcoded_version);
    regex version_pattern(R"(^\d\.\d$)");

    if (exists(version_file)) {
        std::ifstream version_f(version_file);
        std::getline(version_f, version);
        if(!regex_match(version, version_pattern)){
            version = k_hardcoded_version;
            printf(FMT_STR_INVALID_CONFIG_VERSION_NUM, version_file.string().c_str(), k_hardcoded_version);
        } else {
            printf(FMT_STR_SUCCESS_VERSION_READ, version_file.string().c_str(), version.c_str());
        }
    } else {
        printf(FMT_STR_NO_VERSION_FILE, version_file.string().c_str(), k_hardcoded_version);
    }

    entry(input_dir,
          argv[2],
          exec_parent,
          version
          );

    return 0;
}