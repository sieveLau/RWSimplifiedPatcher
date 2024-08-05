#pragma once
#include "DefInfo.h"
#include <filesystem>
#include <fmt/format.h>
#include <iostream>
#include <list>
#include <map>
#include <set>
#include <string>
#include <tinyxml2.h>
#include <memory>

rimtrans::DefInfo a_def(const tinyxml2::XMLElement *node, const std::filesystem::path &file_path,
                        const std::set<std::string> &interested_tags,
                        const std::set<std::string> &list_tags);

auto scan_def(const std::string &src_path, const std::set<std::string> &interested_tags,
              const std::set<std::string> &list_tags,
              std::map<std::string, std::list<rimtrans::DefInfo>> *def_map_by_class = nullptr)
    -> std::map<std::string, std::list<rimtrans::DefInfo>> *;

auto format_def_to_xml_element(const rimtrans::DefInfo& definfo, tinyxml2::XMLElement& root) ->tinyxml2::XMLElement&;

// Just a "no-indent" printer
class custom_printer : public tinyxml2::XMLPrinter {
  public:
    explicit custom_printer(FILE* fp) : tinyxml2::XMLPrinter(fp) {}
  protected:
    void PrintSpace( int depth ) override {
        Print("");
    }
};

void format_defs_to_file(const std::filesystem::path& translation_mod_root_path, const std::map<std::string, std::list<rimtrans::DefInfo>>& def_map_by_class, const std::string& lang = "ChineseSimplified");

void copy_included_trans(const std::filesystem::path& translation_mod_root_path, const std::filesystem::path& original_mod_root_path, const std::string& lang = "ChineseSimplified");

void auto_about(const std::filesystem::path& translation_mod_root_path, const std::filesystem::path& original_mod_root_path, const std::string& version);